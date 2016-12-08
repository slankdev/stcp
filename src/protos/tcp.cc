

#include <assert.h>
#include <stcp/protos/tcp.h>
#include <stcp/mempool.h>
#include <stcp/config.h>
#include <stcp/arch/dpdk/device.h>
#define UNUSED(x) (void)(x)

namespace slank {

size_t tcp_module::mss = 1460;

/*
 * msg: must point iphdr
 */
static tcpip* mtod_tih(mbuf* msg)
{
    return mbuf_mtod<tcpip*>(msg);
}
static uint16_t cksum_tih(tcpip* tih)
{
    return ipv4_udptcp_cksum(&tih->ip, &tih->tcp);
}
static uint16_t data_len(const tcpip* tih)
{
    uint16_t iptotlen = ntoh16(tih->ip.total_length);
    uint16_t iphlen = (tih->ip.version_ihl & 0x0f)<<2;
    uint16_t tcphlen  = ((tih->tcp.data_off>>4)<<2);
    return iptotlen - iphlen - tcphlen;
}
static uint16_t opt_len(const tcpip* tih)
{
    uint16_t len = tih->tcp.data_off>>2;
    len -= sizeof(stcp_tcp_header);
    return len;
}
static void swap_port(tcpip* tih)
{
    uint16_t tmp   = tih->tcp.sport;
    tih->tcp.sport = tih->tcp.dport;
    tih->tcp.dport = tmp;
}
static bool HAVE(tcpip* tih, tcpflag type)
{
    return ((tih->tcp.flags & type) != 0x00);
}
static const char* tcpstate2str(tcpstate state)
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
static const char* sockstate2str(socketstate state)
{
    switch (state) {
        case SOCKS_USE :    return "USE";
        case SOCKS_UNUSE:   return "UNUSE";
        case SOCKS_WAITACCEPT: return "WAITACCEPT";
        default:            return "UNKNOWN";
    }
}


void tcp_module::init()
{
    mp = pool_create(
            "TCP Mem Pool",
            8192 * eth_dev_count(),
            250,
            0,
            MBUF_DEFAULT_BUF_SIZE,
            cpu_socket_id());
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
    parent(nullptr),
    wait_accept_count(0),
    sock_state(SOCKS_UNUSE),
    tcp_state(TCPS_CLOSED),
    port(0),
    pair_port(0),
    si(0, 0)
{
    init();
}

stcp_tcp_sock::~stcp_tcp_sock() {}

void stcp_tcp_sock::init()
{
    parent = nullptr;
    wait_accept_count = 0;
    sock_state = SOCKS_UNUSE;
    tcp_state  = TCPS_CLOSED;
    port = 0;
    pair_port = 0;
    si.iss_H(0);
    si.irs_H(0);
}

void stcp_tcp_sock::term()
{
    while (!rxq.empty()) {
        mbuf_free(rxq.pop());
    }
    while (!txq.empty()) {
        mbuf_free(txq.pop());
    }
}





void stcp_tcp_sock::write(mbuf* msg)
{
    if (tcp_state != TCPS_ESTABLISHED) {
        std::string errstr = "Not Open Port state=";
        errstr += tcpstate2str(tcp_state);
        throw exception(errstr.c_str());
    }
    txq.push(msg);
}


mbuf* stcp_tcp_sock::read()
{
    while (rxq.size() == 0) {
        if (tcp_state == TCPS_CLOSED) {
            std::string errstr = "Not Open Port state=";
            errstr += tcpstate2str(tcp_state);
            throw exception(errstr.c_str());
        }
    }

    mbuf* m = rxq.pop();
    DPRINT("[%15p] READ datalen=%zd\n", this, mbuf_pkt_len(m));
    return m;
}



/*
 * This function blocks until alloc connection.
 */
stcp_tcp_sock* stcp_tcp_sock::accept(struct stcp_sockaddr_in* addr)
{
    UNUSED(addr);

    for (;;) {
        for (stcp_tcp_sock& s : core::tcp.socks) {
            if ((s.parent == this) && (s.sock_state == SOCKS_WAITACCEPT)) {
                DPRINT("[%15p] ACCEPT return new socket [%p]\n", this, &s);
                s.sock_state = SOCKS_USE;
                wait_accept_count--;
                return &s;
            }
        }
    }
}



void stcp_tcp_sock::proc()
{
    while (!txq.empty()) {
        mbuf* msg = txq.pop();
        size_t datalen = mbuf_pkt_len(msg);
        DPRINT("[%15p] proc_ESTABLISHED send(txq.pop(), %zd)\n",
                this, mbuf_pkt_len(msg));

        mbuf_push(msg, sizeof(stcp_ip_header));
        mbuf_push(msg, sizeof(stcp_tcp_header));
        tcpip* tih = mtod_tih(msg);

        /*
         * Craft IP header for tcp checksum
         */
        tih->ip.total_length  = hton16(mbuf_pkt_len(msg));
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
        si.snd_nxt_H(si.snd_nxt_H() + datalen);

    }
}


void tcp_module::proc()
{
    for (size_t i=0; i<socks.size(); i++) {
        socks[i].proc();
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
    wait_accept_count = 0;
    max_connect   = backlog;
    move_state(TCPS_LISTEN);
}


void stcp_tcp_sock::move_state(tcpstate next_state)
{
    DPRINT("[%15p] %s -> %s \n", this,
            tcpstate2str(tcp_state),
            tcpstate2str(next_state) );

    switch (tcp_state) {
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
    if (next_state == TCPS_CLOSED) {
        sock_state = SOCKS_UNUSE;
    }
}


void stcp_tcp_sock::move_state_from_CLOSED(tcpstate next_state)
{
    switch (next_state) {
        case TCPS_LISTEN:
        case TCPS_SYN_SENT:
            tcp_state = next_state;
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
            tcp_state = next_state;
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
            tcp_state = next_state;
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
            tcp_state = next_state;
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
            tcp_state = next_state;
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
            tcp_state = next_state;
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
            tcp_state = next_state;
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
            tcp_state = next_state;
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
            tcp_state = next_state;
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
            tcp_state = next_state;
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
            tcp_state = next_state;
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
            = mbuf_mtod_offset<stcp_tcp_header*>(msg, sizeof(stcp_ip_header));
        uint8_t* buf = reinterpret_cast<uint8_t*>(th);
        buf += sizeof(stcp_tcp_header);
        size_t tcpoplen = th->data_off/4 - sizeof(stcp_tcp_header);
        memset(buf, 0x00, tcpoplen);
    }

    switch (tcp_state) {
        case TCPS_CLOSED:
            rx_push_CLOSED(mbuf_clone(msg, core::tcp.mp), src);
            break;
        case TCPS_LISTEN:
            rx_push_LISTEN(mbuf_clone(msg, core::tcp.mp), src);
            break;
        case TCPS_SYN_SENT:
            rx_push_SYN_SEND(mbuf_clone(msg, core::tcp.mp), src);
            break;
        case TCPS_SYN_RCVD:
        case TCPS_ESTABLISHED:
        case TCPS_FIN_WAIT_1:
        case TCPS_FIN_WAIT_2:
        case TCPS_CLOSE_WAIT:
        case TCPS_CLOSING:
        case TCPS_LAST_ACK:
        case TCPS_TIME_WAIT:
            rx_push_ELSESTATE(mbuf_clone(msg, core::tcp.mp), src);
            break;
        default:
            throw exception("OKASHII91934");
    }
}



void stcp_tcp_sock::rx_push_CLOSED(mbuf* msg, stcp_sockaddr_in* src)
{
    UNUSED(src);
    tcpip* tih = mtod_tih(msg);

    if (HAVE(tih, TCPF_RST)) {
        mbuf_free(msg);
    } else {
#if 1
        mbuf_free(msg);
        return;
#else
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
#endif
    }
}


void stcp_tcp_sock::rx_push_LISTEN(mbuf* msg, stcp_sockaddr_in* src)
{
    tcpip* tih = mtod_tih(msg);

    /*
     * 1: RST Check
     */
    if (HAVE(tih, TCPF_RST)) {
        mbuf_free(msg);
        return;
    }

    /*
     * 2: ACK Check
     */
    if (HAVE(tih, TCPF_ACK)) {
        mbuf_free(msg);
        return;
    }

    /*
     * 3: SYN Check
     */
    if (HAVE(tih, TCPF_SYN)) {
        /*
         * Be not implemented
         *  - Securty Check
         *  - Priority Check
         * TODO: imple
         */
        stcp_tcp_sock* newsock = core::create_tcp_socket();
        newsock->tcp_state = TCPS_SYN_RCVD;
        newsock->sock_state = SOCKS_WAITACCEPT;
        newsock->port      = port;
        newsock->pair_port = tih->tcp.sport;
        newsock->si.iss_H(rand() % 0xffffffff);
        newsock->si.irs_N(tih->tcp.seq);
        newsock->parent = this;
        stcp_printf("[%15p] open new connection from %p \n", newsock, this);

        wait_accept_count ++;

        newsock->addr.sin_addr = tih->ip.dst;
        newsock->pair.sin_addr = tih->ip.src;

        newsock->si.rcv_nxt_H(ntoh32(tih->tcp.seq) + 1);

        swap_port(tih);
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
                swap_port(tih);
                tih->tcp.seq   = tih->tcp.ack;
                tih->tcp.flags = TCPF_RST;

                core::tcp.tx_push(msg, src);
            } else {
                mbuf_free(msg);
            }
            DPRINT("SLANKDEVSLANKDEV error: connection reset\n");
            move_state(TCPS_CLOSED);
            return;
        }
    }

    /*
     * 2: RST Check
     */
    if (HAVE(tih, TCPF_RST)) {
        mbuf_free(msg);
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
        return;
    }

    /*
     * 5: (!SYN && !RST) Pattern
     */
    if (!HAVE(tih, TCPF_SYN) && !HAVE(tih, TCPF_RST)) {
        mbuf_free(msg);
        return;
    }

    mbuf_free(msg);
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
    if (!rx_push_ES_seqchk(mbuf_clone(msg, core::tcp.mp), src))  return;
    if (!rx_push_ES_rstchk(mbuf_clone(msg, core::tcp.mp), src))  return;

    /*
     * 3: Securty and Priority Check
     * TODO: not implement yet
     */

    if (!rx_push_ES_synchk(mbuf_clone(msg, core::tcp.mp), src))  return;
    if (!rx_push_ES_ackchk(mbuf_clone(msg, core::tcp.mp), src))  return;

    /*
     * 6: URG Check
     * TODO: not implement yet
     */

    if (!rx_push_ES_textseg(mbuf_clone(msg, core::tcp.mp), src)) return;
    if (!rx_push_ES_finchk( mbuf_clone(msg, core::tcp.mp), src))  return;
}


/*
 * 1: Sequence Number Check
 */
bool stcp_tcp_sock::rx_push_ES_seqchk(mbuf* msg, stcp_sockaddr_in* src)
{
    UNUSED(src);
    tcpip* tih = mtod_tih(msg);
    switch (tcp_state) {

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
                    uint32_t seq  = ntoh32(tih->tcp.seq);
                    uint32_t rnxt = si.rcv_nxt_H();
                    uint32_t rwin = si.rcv_win_H();
                    uint32_t seqdlen = seq+data_len(tih)-1;

                    bool cond1 = (rnxt <= seq) && (seq <= rnxt + rwin);
                    bool cond2 = rnxt <= seqdlen && seqdlen < rnxt + rwin;
                    pass = cond1 || cond2;
                }
            }

            if (!pass) {
                mbuf_free(msg);
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
    switch (tcp_state) {
        case TCPS_SYN_RCVD:
        {
            if (HAVE(tih, TCPF_RST)) {
                DPRINT("SLANKDEVSLANKDEV conection reset\n");
                mbuf_free(msg);
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
                DPRINT("SLANKDEVSLANKDEV conection reset\n");
                mbuf_free(msg);
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
                mbuf_free(msg);
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
    switch (tcp_state) {
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
                DPRINT("SLANKDEVSLANKDEV conection reset\n");
                mbuf_free(msg);
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
        switch (tcp_state) {
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

                if (tcp_state == TCPS_CLOSING) {
                    if (si.snd_nxt_H() <= ntoh32(tih->tcp.ack)) {
                        move_state(TCPS_TIME_WAIT);
                    }
                    mbuf_free(msg);
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
        mbuf_free(msg);
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
        switch (tcp_state) {
            case TCPS_ESTABLISHED:
            case TCPS_FIN_WAIT_1:
            case TCPS_FIN_WAIT_2:
            {
                mbuf* mseg = mbuf_clone(msg, core::tcp.mp);
                mbuf_pull(mseg, sizeof(stcp_ip_header));
                uint16_t tcphlen  = ((tih->tcp.data_off>>4)<<2);
                mbuf_pull(mseg, tcphlen);
                rxq.push(mseg);

                si.rcv_nxt_inc_H(data_len(tih));
                mbuf_trim(msg, data_len(tih));

                tih->ip.dst = tih->ip.src;
                tih->ip.src = addr.sin_addr;
                tih->ip.next_proto_id = STCP_IPPROTO_TCP;
                tih->ip.total_length  = hton16(mbuf_pkt_len(msg));

                swap_port(tih);
                tih->tcp.seq    = si.snd_nxt_N();
                tih->tcp.ack    = si.rcv_nxt_N();
                tih->tcp.flags  = TCPF_ACK;
                tih->tcp.rx_win = si.snd_win_N();
                tih->tcp.urp    = 0x0000;
                tih->tcp.cksum  = 0x0000;

                tih->tcp.cksum   = cksum_tih(tih);
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
        switch (tcp_state) {
            case TCPS_CLOSED:
            case TCPS_LISTEN:
            case TCPS_SYN_SENT:
                mbuf_free(msg);
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
        stcp_printf("[%15p] connection closing\n", this);
        si.rcv_nxt_H(ntoh32(tih->tcp.seq) + 1);

        swap_port(tih);
        tih->tcp.seq = si.snd_nxt_N();
        tih->tcp.ack = si.rcv_nxt_N();

        if (tcp_state == TCPS_CLOSE_WAIT) {
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
    s.write("\tsock/tcp=%s/%s [this=%p]",
            sockstate2str(sock_state),
            tcpstate2str(tcp_state),
            this);

    switch (tcp_state) {
        case TCPS_LISTEN:
            s.write("\t - local  port: %u", ntoh16(port));
            s.write("\t - wait accept count: %zd", wait_accept_count);
            break;
        case TCPS_ESTABLISHED:
            s.write("\t - local : %s:%u", addr.c_str(), ntoh16(port)     );
            s.write("\t - remote: %s:%u", pair.c_str(), ntoh16(pair_port));
            s.write("\t - txq/rxq: %zd/%zd",rxq.size(), txq.size());
            s.write("\t - stream info");
            s.write("\t   - iss    : %u", si.iss_H());
            s.write("\t   - snd_una: %u", si.snd_una_H());
            s.write("\t   - snd_nxt: %u", si.snd_nxt_H());
            s.write("\t   - snd_win: %u", si.snd_win_H());
            // s.write("\t - snd_up : %u", si.snd_up_H() );
            s.write("\t   - snd_wl1: %u", si.snd_wl1_H());
            s.write("\t   - snd_wl2: %u", si.snd_wl2_H());
            s.write("\t   - irs    : %u", si.irs_H()    );
            s.write("\t   - rcv_nxt: %u", si.rcv_nxt_H());
            s.write("\t   - rcv_wnd: %u", si.rcv_win_H());
            // s.write("\t - rcv_up : %u", si.rcv_up_H() );
            break;
        default:
            break;
    }
}


void tcp_module::print_stat() const
{
    stat& s = stat::instance();
    s.write("TCP module");

    if (!socks.empty()) {
        s.write("");
        s.write("\tNetStat %zd ports", socks.size());
    }
    for (size_t i=0; i<socks.size(); i++) {
        socks[i].print_stat();
    }
    s.write("\n\n\n\n");
    s.write("\n\n\n\n");
}



void tcp_module::rx_push(mbuf* msg, stcp_sockaddr_in* src)
{
    stcp_tcp_header* th = mbuf_mtod<stcp_tcp_header*>(msg);

    bool find_socket = false;
    uint16_t dst_port = th->dport;
    for (stcp_tcp_sock& sock : socks) {
        if (sock.port == dst_port) {
            mbuf* m = mbuf_clone(msg, core::tcp.mp);
            mbuf_push(m, sizeof(stcp_ip_header));
            sock.rx_push(m, src);
            find_socket = true;
        }
    }

    if (!find_socket) {
        mbuf_push(msg, sizeof(stcp_ip_header));
        tcpip* tih = mtod_tih(msg);

        /*
         * Delete TCP Option field
         */
        mbuf_trim(msg, opt_len(tih));

        /*
         * Set TCP/IP hdr
         */
        tih->ip.src           = tih->ip.dst;
        tih->ip.dst           = src->sin_addr;
        tih->ip.next_proto_id = STCP_IPPROTO_TCP;
        tih->ip.total_length  = hton16(mbuf_pkt_len(msg));
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
    mbuf_free(msg);
}




} /* namespace slank */
