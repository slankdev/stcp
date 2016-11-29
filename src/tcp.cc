

#include <assert.h>
#include <stcp/tcp.h>
#include <stcp/config.h>
#define UNUSED(x) (void)(x)

namespace slank {

size_t tcp_module::mss = 1460;

/*
 * msg: must point iphdr
 */
inline tcpip* mtod_tih(mbuf* msg)
{
    return rte::pktmbuf_mtod<tcpip*>(msg);
}
inline uint16_t cksum_tih(tcpip* h)
{
    return rte_ipv4_udptcp_cksum(
        reinterpret_cast<ipv4_hdr*>(&h->ip), &h->tcp);
}
inline uint16_t data_len(const stcp_tcp_header* th,
        const stcp_ip_header* ih)
{
    uint16_t iptotlen = ntoh16(ih->total_length);
    uint16_t iphlen = (ih->version_ihl & 0x0f)<<2;
    uint16_t tcphlen  = ((th->data_off>>4)<<2);
    return iptotlen - iphlen - tcphlen;
}
inline uint16_t data_len(const tcpip* tih)
{
    return data_len(&tih->tcp, &tih->ip);
}
inline uint16_t opt_len(const tcpip* tih)
{
    uint16_t len = tih->tcp.data_off>>2;
    len -= sizeof(stcp_tcp_header);
    return len;
}
inline void swap_port(stcp_tcp_header* th)
{
    uint16_t tmp = th->sport;
    th->sport    = th->dport;
    th->dport    = tmp;
}
inline void swap_port(tcpip* tih)
{
    swap_port(&tih->tcp);
}
inline bool HAVE(stcp_tcp_header* th, tcpflag type)
{
    return ((th->flags & type) != 0x00);
}
inline bool HAVE(tcpip* tih, tcpflag type)
{
    return HAVE(&tih->tcp, type);
}
inline const char* tcpstate2str(tcpstate state)
{
    switch (state) {
        case TCPS_CLOSED:      return "CLOSED";
        case TCPS_LISTEN:      return "LISTEN";
        case TCPS_SYN_SENT:    return "SYN_SENT";
        case TCPS_SYN_RCVD:    return "SYN_RCVD";
        case TCPS_ESTABLISHED: return "ESTABLISHED";
        case TCPS_FIN_WAIT_1:  return "FIN_WAIT_1";
        case TCPS_FIN_WAIT_2:  return "FIN_WAIT_2";
        case TCPS_CLOSE_WAIT:  return "CLOSE_WAIT";
        case TCPS_CLOSING:     return "CLOSING";
        case TCPS_LAST_ACK:    return "LAST_ACK";
        case TCPS_TIME_WAIT:   return "TIME_WAIT";
        default:               return "UNKNOWN";
    }
}


/*
 * msg's head must points ip-header
 */
void tcp_module::tx_push(mbuf* msg, const stcp_sockaddr_in* dst)
{
    mbuf_pull(msg, sizeof(stcp_ip_header));
    core::ip.tx_push(msg, dst, STCP_IPPROTO_TCP);
}


stcp_tcp_sock::stcp_tcp_sock() :
    accepted(false),
    dead(false),
    head(nullptr),
    num_connected(0),
    state(TCPS_CLOSED),
    port(0),
    pair_port(0),
    si(0, 0)
{
    DEBUG("[%15p] SOCK CNSTRCTR \n", this);
}

/*
 * Constructor for LISTEN socket
 */
stcp_tcp_sock::stcp_tcp_sock(tcpstate s, uint16_t lp, uint16_t rp,
            uint32_t arg_iss, uint32_t arg_irs, stcp_tcp_sock* h) :
    accepted(false),
    dead(false),
    head(h),
    num_connected(0),
    state(s),
    port(lp),
    pair_port(rp),
    si(arg_iss, arg_irs)
{
    DEBUG("[%15p] SOCK CNSTRCTR(%s,%u,%u,%u,%u,%p) \n",
            this, tcpstate2str(state), port, pair_port, si.iss_H(), si.irs_H(), head);
}

stcp_tcp_sock::~stcp_tcp_sock()
{
    DEBUG("[%15p] SOCK DESTRUCTOR \n", this);
    if (head) {
        head->num_connected --;
    }
}

stcp_tcp_sock* stcp_tcp_sock::alloc_new_sock_connected(tcpstate st,
        uint16_t lp, uint16_t rp, uint32_t arg_iss, uint32_t arg_irs,
        stcp_tcp_sock* head)
{
    stcp_tcp_sock* s = new stcp_tcp_sock(st, lp, rp, arg_iss, arg_irs, head);
    core::tcp.socks.push_back(s);
    return s;
}




void stcp_tcp_sock::write(mbuf* msg)
{
    if (state != TCPS_ESTABLISHED) {
        std::string errstr = "Not Open Port state=";
        errstr += tcpstate2str(state);
        throw exception(errstr.c_str());
    }
    txq.push(msg);
}


mbuf* stcp_tcp_sock::read()
{
    while (rxq.size() == 0) {
        if (state == TCPS_CLOSED) {
            std::string errstr = "Not Open Port state=";
            errstr += tcpstate2str(state);
            throw exception(errstr.c_str());
        }
    }

    mbuf* m = rxq.pop();
    DEBUG("[%15p] READ datalen=%zd\n", this, rte::pktmbuf_pkt_len(m));
    return m;
}



/*
 * This function blocks until alloc connection.
 */
stcp_tcp_sock* stcp_tcp_sock::accept(struct stcp_sockaddr_in* addr)
{
    UNUSED(addr);

    while (wait_accept.size() == 0) ;

    /*
     * Dequeue wait_accept and return that.
     */
    stcp_tcp_sock* sock = wait_accept.pop();
    DEBUG("[%15p] ACCEPT return new socket [%p]\n", this, sock);
    return sock;
}



void stcp_tcp_sock::proc()
{

    /*
     * TODO
     * Proc msgs from txq to tcp_module
     */


    /*
     * Othre proc;
     */
    switch (state) {
        case TCPS_CLOSE_WAIT:
            break;
        case TCPS_ESTABLISHED:
            proc_ESTABLISHED();
            break;
        case TCPS_LISTEN:
        case TCPS_CLOSED:
        case TCPS_SYN_SENT:
        case TCPS_SYN_RCVD:
        case TCPS_FIN_WAIT_1:
        case TCPS_FIN_WAIT_2:
        case TCPS_CLOSING:
        case TCPS_LAST_ACK:
        case TCPS_TIME_WAIT:
            /*
             * TODO
             * Not Implement yet.
             * No Operation
             */
            break;
        default:
            throw exception("UNKNOWN TCP STATE!!!");
            break;
    }
}


void stcp_tcp_sock::proc_ESTABLISHED()
{
    while (!txq.empty()) {
        mbuf* msg = txq.pop();
        size_t datalen = rte::pktmbuf_pkt_len(msg);
        DEBUG("[%15p] proc_ESTABLISHED send(txq.pop(), %zd)\n",
                this, rte::pktmbuf_pkt_len(msg));

        tcpip* tih = mtod_tih(msg);

        /*
         * Craft IP header for tcp checksum
         */
        tih->ip.total_length  = hton16(rte::pktmbuf_pkt_len(msg));
        tih->ip.next_proto_id = STCP_IPPROTO_TCP;
        tih->ip.src           = addr.sin_addr;
        tih->ip.dst           = pair.sin_addr;

        /*
         * Craft TCP header
         */
        tih->tcp.sport    = port     ;
        tih->tcp.dport    = pair_port;
        tih->tcp.seq      = si.snd_nxt_N();
        tih->tcp.ack      = si.rcv_nxt_N();
        tih->tcp.data_off = sizeof(stcp_tcp_header) >> 2 << 4;
        tih->tcp.flags    = TCPF_PSH|TCPF_ACK;
        tih->tcp.rx_win   = si.snd_win_N();
        tih->tcp.cksum    = 0x0000;
        tih->tcp.urp      = 0; // TODO hardcode

        tih->tcp.cksum    = cksum_tih(tih);

        /*
         * send to ip module
         */
        core::tcp.tx_push(msg, &pair);

        /*
         * TODO KOKOJANAKUNE
         */
        si.snd_nxt_H(si.snd_nxt_H() + datalen);

    }
}


void tcp_module::proc()
{
    for (size_t i=0; i<socks.size(); i++) {
        socks[i]->proc();
    }
}

void stcp_tcp_sock::bind(const struct stcp_sockaddr_in* addr, size_t addrlen)
{
    if (addrlen < sizeof(sockaddr_in))
        throw exception("Invalid addrlen");
    port = addr->sin_port;
}
void stcp_tcp_sock::listen(size_t backlog)
{
    if (backlog < 1) throw exception("OKASHII1944");
    num_connected = 0;
    max_connect   = backlog;
    move_state(TCPS_LISTEN);
}


void stcp_tcp_sock::move_state_DEBUG(tcpstate next_state)
{
    DEBUG("[%15p] %s -> %s (MOVE state debug) \n", this,
            tcpstate2str(state),
            tcpstate2str(next_state) );
    state = next_state;
}

void stcp_tcp_sock::move_state(tcpstate next_state)
{
// TODO
#if 1
    DEBUG("[%15p] %s -> %s \n", this,
            tcpstate2str(state),
            tcpstate2str(next_state) );
#endif

    switch (state) {
        case TCPS_CLOSED     :
            move_state_from_CLOSED(next_state);
            break;
        case TCPS_LISTEN     :
            move_state_from_LISTEN(next_state);
            break;
        case TCPS_SYN_SENT   :
            move_state_from_SYN_SENT(next_state);
            break;
        case TCPS_SYN_RCVD   :
            move_state_from_SYN_RCVD(next_state);
            break;
        case TCPS_ESTABLISHED:
            move_state_from_ESTABLISHED(next_state);
            break;
        case TCPS_FIN_WAIT_1 :
            move_state_from_FIN_WAIT_1(next_state);
            break;
        case TCPS_FIN_WAIT_2 :
            move_state_from_FIN_WAIT_2(next_state);
            break;
        case TCPS_CLOSE_WAIT :
            move_state_from_CLOSE_WAIT(next_state);
            break;
        case TCPS_CLOSING    :
            move_state_from_CLOSING(next_state);
            break;
        case TCPS_LAST_ACK   :
            move_state_from_LAST_ACK(next_state);
            break;
        case TCPS_TIME_WAIT  :
            move_state_from_TIME_WAIT(next_state);
            break;
        default:
            throw exception("invalid tcp sock state");
            break;
    }
}


void stcp_tcp_sock::move_state_from_CLOSED(tcpstate next_state)
{
    switch (next_state) {
        case TCPS_LISTEN:
        case TCPS_SYN_SENT:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_LISTEN(tcpstate next_state)
{
    switch (next_state) {
        case TCPS_CLOSED:
        case TCPS_SYN_SENT:
        case TCPS_SYN_RCVD:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_SYN_SENT(tcpstate next_state)
{
    switch (next_state) {
        case TCPS_CLOSED:
        case TCPS_SYN_RCVD:
        case TCPS_ESTABLISHED:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_SYN_RCVD(tcpstate next_state)
{
    switch (next_state) {
        case TCPS_ESTABLISHED:
        case TCPS_FIN_WAIT_1:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_ESTABLISHED(tcpstate next_state)
{
    switch (next_state) {
        case TCPS_FIN_WAIT_1:
        case TCPS_CLOSE_WAIT:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_FIN_WAIT_1(tcpstate next_state)
{
    switch (next_state) {
        case TCPS_CLOSING:
        case TCPS_FIN_WAIT_2:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_FIN_WAIT_2(tcpstate next_state)
{
    switch (next_state) {
        case TCPS_TIME_WAIT:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_CLOSE_WAIT(tcpstate next_state)
{
    switch (next_state) {
        case TCPS_LAST_ACK:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_CLOSING(tcpstate next_state)
{
    switch (next_state) {
        case TCPS_TIME_WAIT:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_LAST_ACK(tcpstate next_state)
{
    switch (next_state) {
        case TCPS_CLOSED:
            state = next_state;
            dead = true;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_TIME_WAIT(tcpstate next_state)
{
    switch (next_state) {
        case TCPS_CLOSED:
            state = next_state;
            dead = true;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}




/*
 * msg: points ip_header
 */
void stcp_tcp_sock::rx_push(mbuf* msg,stcp_sockaddr_in* src)
{
    {
        /*
         * TODO Proc TCP-Option
         * ERASE zeroclear tcp options
         */
        stcp_tcp_header* th
            = rte::pktmbuf_mtod_offset<stcp_tcp_header*>(msg, sizeof(stcp_ip_header));
        uint8_t* buf = reinterpret_cast<uint8_t*>(th);
        buf += sizeof(stcp_tcp_header);
        size_t tcpoplen = th->data_off/4 - sizeof(stcp_tcp_header);
        memset(buf, 0x00, tcpoplen);
    }

    switch (state) {
        case TCPS_CLOSED:
            rx_push_CLOSED(msg, src);
            break;
        case TCPS_LISTEN:
            rx_push_LISTEN(msg, src);
            break;
        case TCPS_SYN_SENT:
            rx_push_SYN_SEND(msg, src);
            break;
        case TCPS_SYN_RCVD:
        case TCPS_ESTABLISHED:
        case TCPS_FIN_WAIT_1:
        case TCPS_FIN_WAIT_2:
        case TCPS_CLOSE_WAIT:
        case TCPS_CLOSING:
        case TCPS_LAST_ACK:
        case TCPS_TIME_WAIT:
            rx_push_ELSESTATE(msg, src);
            break;
        default:
            throw exception("OKASHII91934");
    }
}



void stcp_tcp_sock::rx_push_CLOSED(mbuf* msg, stcp_sockaddr_in* src)
{
    tcpip* tih = mtod_tih(msg);

    if (HAVE(tih, TCPF_RST)) {
        rte::pktmbuf_free(msg);
    } else {
        if (HAVE(tih, TCPF_ACK)) {
            swap_port(&tih->tcp);
            tih->tcp.ack = tih->tcp.seq + hton32(data_len(tih));
            tih->tcp.seq = 0;
            tih->tcp.flags   = TCPF_RST|TCPF_ACK;
        } else {
            swap_port(&tih->tcp);
            tih->tcp.seq = tih->tcp.ack;
            tih->tcp.flags   = TCPF_RST;
        }
        tih->tcp.cksum     = 0x0000;
        tih->tcp.urp       = 0x0000;

        tih->tcp.cksum = cksum_tih(tih);

        core::tcp.tx_push(msg, src);
    }
    return;
}


void stcp_tcp_sock::rx_push_LISTEN(mbuf* msg, stcp_sockaddr_in* src)
{
    tcpip* tih = mtod_tih(msg);

    /*
     * 1: RST Check
     */
    if (HAVE(tih, TCPF_RST)) {
        rte::pktmbuf_free(msg);
        return;
    }

    /*
     * 2: ACK Check
     */
    if (HAVE(tih, TCPF_ACK)) {
        rte::pktmbuf_free(msg);
        return;
    }

    /*
     * 3: SYN Check
     */
    if (HAVE(tih, TCPF_SYN)) {
        /*
         * Securty Check is not implemented
         * TODO: not imple yet
         */

        /*
         * Priority Check
         * TODO: not imple yet
         */

        stcp_tcp_sock* newsock = alloc_new_sock_connected(
                TCPS_SYN_RCVD,
                port, tih->tcp.sport,
                rte::rand() % 0xffffffff,
                hton32(tih->tcp.seq),
                this);
        num_connected ++;
        wait_accept.push(newsock);

        newsock->addr.sin_addr = tih->ip.dst;
        newsock->pair.sin_addr = tih->ip.src;

        newsock->si.rcv_nxt_H(ntoh32(tih->tcp.seq) + 1);

        swap_port(&tih->tcp);
        tih->tcp.seq     = newsock->si.iss_N();
        tih->tcp.ack     = newsock->si.rcv_nxt_N();
        tih->tcp.flags   = TCPF_SYN|TCPF_ACK;
        tih->tcp.rx_win  = newsock->si.snd_win_N();
        tih->tcp.urp     = 0x0000; // TODO hardcode
        tih->tcp.cksum   = 0x0000;

        tih->tcp.cksum   = cksum_tih(tih);

        core::tcp.tx_push(msg, src);

        newsock->si.snd_nxt_H(newsock->si.iss_H() + 1);
        newsock->si.snd_una_N(newsock->si.iss_N());
        return;
    }

    /*
     * 4: Else Text Control
     */
    throw exception("OKASHII93931");
}


void stcp_tcp_sock::rx_push_SYN_SEND(mbuf* msg, stcp_sockaddr_in* src)
{
    tcpip* tih = mtod_tih(msg);

    /*
     * 1: ACK Check
     */
    if (HAVE(tih, TCPF_ACK)) {
        if (ntoh32(tih->tcp.ack) <= si.iss_H() ||
                ntoh32(tih->tcp.ack) > si.snd_nxt_H()) {
            if (HAVE(tih, TCPF_RST)) {
                swap_port(&tih->tcp);
                tih->tcp.seq   = tih->tcp.ack;
                tih->tcp.flags = TCPF_RST;

                core::tcp.tx_push(msg, src);
            } else {
                rte::pktmbuf_free(msg);
            }
            printf("SLANKDEVSLANKDEV error: connection reset\n");
            move_state(TCPS_CLOSED);
            return;
        }
    }

    /*
     * 2: RST Check
     */
    if (HAVE(tih, TCPF_RST)) {
        rte::pktmbuf_free(msg);
        return;
    }

    /*
     * 3: Securty and Priority Check
     * TODO not implement yet
     */

    /*
     * 4: SYN Check
     */
    assert(!HAVE(tih, TCPF_ACK) && !HAVE(tih, TCPF_RST));

    if (HAVE(tih, TCPF_SYN)) {
        si.rcv_nxt_H(ntoh32(tih->tcp.seq) + 1);
        si.irs_N(tih->tcp.seq);

        if (HAVE(tih, TCPF_ACK)) {
            si.snd_una_N(tih->tcp.ack);
        }

        if (si.snd_una_H() > si.iss_H()) {
            move_state(TCPS_ESTABLISHED);
            swap_port(tih);
            tih->tcp.seq   = si.snd_nxt_N();
            tih->tcp.ack   = si.rcv_nxt_N();
            tih->tcp.flags = TCPF_ACK;
        } else {
            move_state(TCPS_SYN_RCVD);
            swap_port(tih);
            tih->tcp.seq   = si.iss_N();
            tih->tcp.ack   = si.rcv_nxt_N();
            tih->tcp.flags = TCPF_SYN|TCPF_ACK;
        }
        core::tcp.tx_push(msg, src);
    }

    /*
     * 5: (!SYN && !RST) Pattern
     */
    if (!HAVE(tih, TCPF_SYN) && !HAVE(tih, TCPF_RST)) {
        rte::pktmbuf_free(msg);
        return;
    }

    throw exception("OKASHII12222223");
}



/*
 * rx_push_XXXX()
 * - TCPS_SYN_RCVD:
 * - TCPS_ESTABLISHED:
 * - TCPS_FIN_WAIT_1:
 * - TCPS_FIN_WAIT_2:
 * - TCPS_CLOSE_WAIT:
 * - TCPS_CLOSING:
 * - TCPS_LAST_ACK:
 * - TCPS_TIME_WAIT:
 */
void stcp_tcp_sock::rx_push_ELSESTATE(mbuf* msg, stcp_sockaddr_in* src)
{
    if (!rx_push_ES_seqchk(msg, src))  return;
    DEBUG("[%s] CLEEAKDFDF\n", tcpstate2str(state));
    if (!rx_push_ES_rstchk(msg, src))  return;

    /*
     * 3: Securty and Priority Check
     * TODO: not implement yet
     */

    if (!rx_push_ES_synchk(msg, src))  return;
    if (!rx_push_ES_ackchk(msg, src))  return;

    /*
     * 6: URG Check
     * TODO: not implement yet
     */

    if (!rx_push_ES_textseg(msg, src)) return;
    if (!rx_push_ES_finchk(msg, src))  return;
}


/*
 * 1: Sequence Number Check
 */
bool stcp_tcp_sock::rx_push_ES_seqchk(mbuf* msg, stcp_sockaddr_in* src)
{
    UNUSED(src);
    tcpip* tih = mtod_tih(msg);
    switch (state) {

        case TCPS_SYN_RCVD:
        case TCPS_ESTABLISHED:
        case TCPS_FIN_WAIT_1:
        case TCPS_FIN_WAIT_2:
        case TCPS_CLOSE_WAIT:
        case TCPS_CLOSING:
        case TCPS_LAST_ACK:
        case TCPS_TIME_WAIT:
        {
            bool pass = false;
            if (data_len(tih) == 0) {
                if (tih->tcp.rx_win == 0) {
                    if (tih->tcp.seq == si.rcv_nxt_N()) {
                        pass = true;
                    }
                } else { /* win > 0 */
                    if (si.rcv_nxt_H() <= ntoh32(tih->tcp.seq)
                        && ntoh32(tih->tcp.seq) <= si.rcv_nxt_H()+si.rcv_win_H()) {
                        pass = true;
                    }
                }
            } else { /* data_len > 0 */
                if (tih->tcp.rx_win > 0) {
#if 1
                    bool cond1 = true;
                    bool cond2 = true;
#else

                    bool cond1 = si.rcv_nxt_H() <= ntoh32(tih->tcp.seq) &&
                        ntoh32(tih->tcp.seq) < si.rcv_nxt_H() + si.rcv_win_H();
                    bool cond2 = si.rcv_nxt_H() <=
                        ntoh32(tih->tcp.seq)+data_len(tih)-1 &&
                        ntoh32(tih->tcp.seq)+data_len(tih)-1 <
                            si.rcv_nxt_H() + si.rcv_win_H();
#endif
                    pass = cond1 || cond2;
                    if (cond1 || cond2) {DEBUG("cond1||cond2\n");}

                    if (pass) {DEBUG("pass\n");}
                    else      {DEBUG("unpass\n");}
                }
            }

            DEBUG("[%s] SSSSSS\n", tcpstate2str(state));
            if (!pass) {
                DEBUG("asdfdfdfdfd\n");
                rte::pktmbuf_free(msg);
                return false;
            }

            if (HAVE(tih, TCPF_RST)) {
                rte::pktmbuf_free(msg);
                return false;
            }
            break;
        }

        case TCPS_CLOSED:
        case TCPS_LISTEN:
        case TCPS_SYN_SENT:
            throw exception("OKASHII3343");
        default:
            throw exception("OKASHII334: unknown state");
    }
    return true;
}


/*
 * 2: TCPF_RST Check
 */
bool stcp_tcp_sock::rx_push_ES_rstchk(mbuf* msg, stcp_sockaddr_in* src)
{
    UNUSED(src);
    tcpip* tih = mtod_tih(msg);
    switch (state) {
        case TCPS_SYN_RCVD:
        {
            if (HAVE(tih, TCPF_RST)) {
                printf("SLANKDEVSLANKDEV conection reset\n");
                rte::pktmbuf_free(msg);
                move_state(TCPS_CLOSED);
                return false;
            }
            break;
        }

        case TCPS_ESTABLISHED:
        case TCPS_FIN_WAIT_1:
        case TCPS_FIN_WAIT_2:
        case TCPS_CLOSE_WAIT:
        {
            if (HAVE(tih, TCPF_RST)) {
                printf("SLANKDEVSLANKDEV conection reset\n");
                rte::pktmbuf_free(msg);
                move_state(TCPS_CLOSED);
                return false;
            }
            break;
        }

        case TCPS_CLOSING:
        case TCPS_LAST_ACK:
        case TCPS_TIME_WAIT:
        {
            if (HAVE(tih, TCPF_RST)) {
                rte::pktmbuf_free(msg);
                move_state(TCPS_CLOSED);
                return false;
            }
            break;
        }

        case TCPS_CLOSED:
        case TCPS_LISTEN:
        case TCPS_SYN_SENT:
            throw exception("OKASHII3900");
        default:
            throw exception("OKASHII883: unknown state");
    }
    return true;
}


/*
 * 4: TCPF_SYN Check
 */
bool stcp_tcp_sock::rx_push_ES_synchk(mbuf* msg, stcp_sockaddr_in* src)
{
    UNUSED(src);
    tcpip* tih = mtod_tih(msg);
    switch (state) {
        case TCPS_SYN_RCVD:
        case TCPS_ESTABLISHED:
        case TCPS_FIN_WAIT_1:
        case TCPS_FIN_WAIT_2:
        case TCPS_CLOSE_WAIT:
        case TCPS_CLOSING:
        case TCPS_LAST_ACK:
        case TCPS_TIME_WAIT:
        {
            if (HAVE(tih, TCPF_SYN)) {
                printf("SLANKDEVSLANKDEV conection reset\n");
                rte::pktmbuf_free(msg);
                move_state(TCPS_CLOSED);
                return false;
            }
            break;
        }

        case TCPS_CLOSED:
        case TCPS_LISTEN:
        case TCPS_SYN_SENT:
            throw exception("OKASHII416");
        default:
            throw exception("OKASHII199: unknown state");
    }
    return true;
}


/*
 * 5: TCPF_ACK Check
 */
bool stcp_tcp_sock::rx_push_ES_ackchk(mbuf* msg, stcp_sockaddr_in* src)
{
    tcpip* tih = mtod_tih(msg);
    if (HAVE(tih, TCPF_ACK)) {
        switch (state) {
            case TCPS_SYN_RCVD:
            {
                if (si.snd_una_H() <= ntoh32(tih->tcp.ack) &&
                        ntoh32(tih->tcp.ack) <= si.snd_nxt_H()) {
                    move_state(TCPS_ESTABLISHED);
                } else {
                    swap_port(tih);
                    tih->tcp.seq   = tih->tcp.ack;
                    tih->tcp.flags = TCPF_RST;
                    core::tcp.tx_push(msg, src);
                }
                return false;
                break;
            }

            case TCPS_ESTABLISHED:
            case TCPS_CLOSE_WAIT:
            case TCPS_CLOSING:
            {
                if (si.snd_una_H() < tih->tcp.ack && tih->tcp.ack <= si.snd_nxt_H()) {
                    si.snd_una_N(tih->tcp.ack);
                }

                if (ntoh32(tih->tcp.ack) < si.snd_una_H()) {
                    swap_port(tih);
                    tih->tcp.seq   = tih->tcp.ack;
                    tih->tcp.flags = TCPF_RST;

                    core::tcp.tx_push(msg, src);
                    return false;
                }

                if (si.snd_una_H() < ntoh32(tih->tcp.ack) &&
                        ntoh32(tih->tcp.ack) <= si.snd_nxt_H()) {
                    if ((si.snd_wl1_H() < ntoh32(tih->tcp.seq))
                            || (si.snd_wl1_N() == tih->tcp.seq)
                            || (si.snd_wl2_N() == tih->tcp.ack)) {
                        si.snd_win_N(tih->tcp.rx_win );
                        si.snd_wl1_N(tih->tcp.seq);
                        si.snd_wl2_N(tih->tcp.ack);
                    }
                }

                if (state == TCPS_CLOSING) {
                    if (si.snd_nxt_H() <= ntoh32(tih->tcp.ack)) {
                        move_state(TCPS_TIME_WAIT);
                    }
                    rte::pktmbuf_free(msg);
                }
                break;
            }

            case TCPS_FIN_WAIT_1:
            {
                move_state(TCPS_FIN_WAIT_2);
                break;
            }
            case TCPS_FIN_WAIT_2:
            {
                printf("OK\n");
                break;
            }
            case TCPS_LAST_ACK:
            {
                if (si.snd_nxt_H() <= ntoh32(tih->tcp.ack)) {
                    move_state(TCPS_CLOSED);
                    return false;
                }
                break;
            }
            case TCPS_TIME_WAIT:
            {
                throw exception("TODO: NOT IMPEL YET");
                break;
            }

            case TCPS_CLOSED:
            case TCPS_LISTEN:
            case TCPS_SYN_SENT:
                throw exception("OKASHII1941");
            default:
                throw exception("OKASHII14001: unknown state");
        }

    } else {
        rte::pktmbuf_free(msg);
        return false;
    }
    return true;
}


/*
 * 7: Text Segment Control
 */
bool stcp_tcp_sock::rx_push_ES_textseg(mbuf* msg, stcp_sockaddr_in* src)
{
    UNUSED(src);
    tcpip* tih = mtod_tih(msg);

    if (data_len(tih) > 0) {
        DEBUG("[%s] SLANKDEVSLANKDEV: asdfadfasdfasfs\n", tcpstate2str(state));
        switch (state) {
            case TCPS_ESTABLISHED:
            case TCPS_FIN_WAIT_1:
            case TCPS_FIN_WAIT_2:
            {
                si.rcv_nxt_inc_H(data_len(tih));
                swap_port(tih);
                tih->tcp.seq   = si.snd_nxt_N();
                tih->tcp.ack   = si.rcv_nxt_N();
                tih->tcp.flags = TCPF_ACK;

                core::tcp.tx_push(msg, src);
                break;
            }

            case TCPS_CLOSE_WAIT:
            case TCPS_CLOSING:
            case TCPS_LAST_ACK:
            case TCPS_TIME_WAIT:

            case TCPS_CLOSED:
            case TCPS_LISTEN:
            case TCPS_SYN_SENT:
                throw exception("OKASHII94010");

            case TCPS_SYN_RCVD:
                throw exception("RFC MITEIGIIIII");

            default:
                throw exception("OKASHII19491: unknown state");
        }
    }
    return true;
}


/*
 * 8: TCPF_FIN Check
 */
bool stcp_tcp_sock::rx_push_ES_finchk(mbuf* msg, stcp_sockaddr_in* src)
{
    UNUSED(src);
    tcpip* tih = mtod_tih(msg);
    if (HAVE(tih, TCPF_FIN)) {
        switch (state) {
            case TCPS_CLOSED:
            case TCPS_LISTEN:
            case TCPS_SYN_SENT:
                rte::pktmbuf_free(msg);
                return false;
                break;
            case TCPS_SYN_RCVD:
            case TCPS_ESTABLISHED:
                move_state(TCPS_CLOSE_WAIT);
                break;
            case TCPS_FIN_WAIT_1:
                move_state(TCPS_CLOSING);
                break;
            case TCPS_FIN_WAIT_2:
                move_state(TCPS_TIME_WAIT);
                break;
            case TCPS_CLOSE_WAIT:
                break;
            case TCPS_CLOSING:
                break;
            case TCPS_LAST_ACK:
                break;
            case TCPS_TIME_WAIT:
                break;

            default:
                throw exception("OKASHII939201: unknown state");
        }
        DEBUG("[%15p] connection closing\n", this);
        si.rcv_nxt_H(ntoh32(tih->tcp.seq) + 1);

        swap_port(tih);
        tih->tcp.seq = si.snd_nxt_N();
        tih->tcp.ack = si.rcv_nxt_N();

        if (state == TCPS_CLOSE_WAIT) {
            tih->tcp.flags = TCPF_ACK|TCPF_FIN;
            move_state(TCPS_LAST_ACK);
        } else {
            tih->tcp.flags = TCPF_ACK;
        }

        tih->tcp.cksum    = 0x0000;
        tih->tcp.urp      = 0x0000;

        tih->tcp.cksum = cksum_tih(tih);
        core::tcp.tx_push(msg, src);

    }
    return true;
}



void stcp_tcp_sock::print_stat() const
{
    stat& s = stat::instance();
    s.write("\t%u/tcp state=%s[this=%p] rx/tx=%zd/%zd %u/%u %u",
            ntoh16(port),
            tcpstate2str(state), this,
            rxq.size(), txq.size(),
            si.snd_nxt_H(), si.rcv_nxt_H(),
            ntoh16(pair_port));

#if 0
    switch (state) {
#if 0
        case TCPS_LISTEN:
            s.write("\t - socket alloced %zd/%zd", num_connected, max_connect);
            s.write("\n\n\n");
            break;
        case TCPS_CLOSED:
            s.write("\n\n\n");
            break;
        default:
            s.write("\n\n\n");
            break;
#endif
        case TCPS_LISTEN:
            s.write("\t - socket alloced %zd/%zd", num_connected, max_connect);
            break;
        case TCPS_ESTABLISHED:
            s.write("\t - iss    : %u", iss    );
            // s.write("\t - snd_una: %u", snd_una);
            s.write("\t - snd_nxt: %u", snd_nxt);
            // s.write("\t - snd_win: %u", snd_win);
            // s.write("\t - snd_up : %u", snd_up );
            // s.write("\t - snd_wl1: %u", snd_wl1);
            // s.write("\t - snd_wl2: %u", snd_wl2);
            s.write("\t - irs    : %u", irs    );
            s.write("\t - rcv_nxt: %u", rcv_nxt);
            // s.write("\t - rcv_wnd: %u", rcv_wnd);
            // s.write("\t - rcv_up : %u", rcv_up );
            s.write("\t - port/pair_port: %u/%u",
                    ntoh16(port),
                    ntoh16(pair_port));
            s.write("\t - addr: %s", addr.c_str());
            s.write("\t - pair: %s", pair.c_str());
            break;
        default:
            break;
    }
#endif
}


void tcp_module::print_stat() const
{
    stat& s = stat::instance();
    s.write("TCP module");
    s.write("\tRX Packets %zd", rx_cnt);
    s.write("\tTX Packets %zd", tx_cnt);

    if (!socks.empty()) {
        s.write("");
        s.write("\tNetStat %zd ports", socks.size());
    }
    for (size_t i=0; i<socks.size(); i++) {
        socks[i]->print_stat();
    }
}



void tcp_module::rx_push(mbuf* msg, stcp_sockaddr_in* src)
{
    stcp_tcp_header* th
        = rte::pktmbuf_mtod<stcp_tcp_header*>(msg);
    rx_cnt++;

    bool find_socket = false;
    uint16_t dst_port = th->dport;
    for (stcp_tcp_sock* sock : socks) {
        if (sock->port == dst_port) {
            mbuf* m = rte::pktmbuf_clone(msg, core::dpdk.get_mempool());
            mbuf_push(m, sizeof(stcp_ip_header));
            sock->rx_push(m, src);
            find_socket = true;
        }
    }

    if (!find_socket) {
        mbuf_push(msg, sizeof(stcp_ip_header));
        tcpip* tih = mtod_tih(msg);

        /*
         * Delete TCP Option field
         */
        rte::pktmbuf_trim(msg, opt_len(tih));

        /*
         * Set TCP/IP hdr
         */
        tih->ip.src           = tih->ip.dst;
        tih->ip.dst           = src->sin_addr;
        tih->ip.next_proto_id = STCP_IPPROTO_TCP;
        tih->ip.total_length  = hton16(rte::pktmbuf_pkt_len(msg));
        swap_port(tih);
        tih->tcp.ack      = tih->tcp.seq + hton32(1);
        tih->tcp.seq      = 0;
        tih->tcp.data_off = sizeof(stcp_tcp_header)/4 << 4;
        tih->tcp.flags    = TCPF_RST|TCPF_ACK;
        tih->tcp.rx_win   = 0;
        tih->tcp.cksum    = 0x0000;
        tih->tcp.urp      = 0x0000;

        tih->tcp.cksum = cksum_tih(tih);
        core::tcp.tx_push(msg, src);
        return;
    }
    rte::pktmbuf_free(msg);
}




} /* namespace slank */
