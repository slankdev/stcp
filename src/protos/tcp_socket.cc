


#include <assert.h>
#include <stcp/protos/tcp.h>
#include <stcp/mempool.h>
#include <stcp/config.h>
#include <stcp/arch/dpdk/device.h>
#include <stcp/protos/tcp_socket.h>
#include <stcp/protos/tcp_util.h>
#define UNUSED(x) (void)(x)

namespace slank {


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
    stcp_printf("[%15p] READ datalen=%zd\n", this, mbuf_pkt_len(m));
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
                stcp_printf("[%15p] ACCEPT return new socket [%p]\n", this, &s);
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
        stcp_printf("[%15p] proc_ESTABLISHED send(txq.pop(), %zd)\n",
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
        tih->tcp.urp      = 0x0000;
        tih->tcp.cksum    = 0x0000;

        tih->tcp.cksum    = cksum_tih(tih);

        /*
         * send to ip module
         */
        core::tcp.tx_push(msg, &pair);
        si.snd_nxt_H(si.snd_nxt_H() + datalen);

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
    stcp_printf("[%15p] %s -> %s \n", this,
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
        term();
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
            mbuf_free(msg);
            throw exception("OKASHII91934");
    }
    mbuf_free(msg);
}



void stcp_tcp_sock::rx_push_CLOSED(mbuf* msg, stcp_sockaddr_in* src)
{
    UNUSED(src);
    tcpip* tih = mtod_tih(msg);

    if (HAVE(tih, TCPF_RST)) {
        mbuf_free(msg);
        return;
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
         * Unsupport belows
         *  - Securty Check
         *  - Priority Check
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
        tih->tcp.urp     = 0x0000;
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
            stcp_printf("error: connection reset\n");
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
    if (!rx_push_ES_seqchk(mbuf_clone(msg, core::tcp.mp), src))  goto drop_packet;
    if (!rx_push_ES_rstchk(mbuf_clone(msg, core::tcp.mp), src))  goto drop_packet;

    /*
     * 3: Securty and Priority Check
     * TODO: not implement yet
     */

    if (!rx_push_ES_synchk(mbuf_clone(msg, core::tcp.mp), src))  goto drop_packet;
    if (!rx_push_ES_ackchk(mbuf_clone(msg, core::tcp.mp), src))  goto drop_packet;

    /*
     * 6: URG Check
     * TODO: not implement yet
     */

    if (!rx_push_ES_textseg(mbuf_clone(msg, core::tcp.mp), src)) goto drop_packet;
    if (!rx_push_ES_finchk( mbuf_clone(msg, core::tcp.mp), src)) goto drop_packet;

drop_packet:
    mbuf_free(msg);

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
            mbuf_free(msg);
            throw exception("OKASHII3343");
        default:
            mbuf_free(msg);
            throw exception("OKASHII334: unknown state");
    }
    mbuf_free(msg);
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
                stcp_printf("conection reset\n");
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
                stcp_printf("conection reset\n");
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
            mbuf_free(msg);
            throw exception("OKASHII3900");
        default:
            mbuf_free(msg);
            throw exception("OKASHII883: unknown state");
    }
    mbuf_free(msg);
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
                stcp_printf("conection reset\n");
                mbuf_free(msg);
                move_state(TCPS_CLOSED);
                return false;
            }
            break;
        }

        case TCPS_CLOSED:
        case TCPS_LISTEN:
        case TCPS_SYN_SENT:
            mbuf_free(msg);
            throw exception("OKASHII416");
        default:
            mbuf_free(msg);
            throw exception("OKASHII199: unknown state");
    }
    mbuf_free(msg);
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
                    mbuf_free(msg);
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
                    mbuf_free(msg);
                    return false;
                }
                break;
            }
            case TCPS_TIME_WAIT:
            {
                mbuf_free(msg);
                throw exception("TODO: NOT IMPEL YET");
                break;
            }

            case TCPS_CLOSED:
            case TCPS_LISTEN:
            case TCPS_SYN_SENT:
                mbuf_free(msg);
                throw exception("OKASHII1941");
            default:
                mbuf_free(msg);
                throw exception("OKASHII14001: unknown state");
        }
        mbuf_free(msg);
        return true;
    } else {
        mbuf_free(msg);
        return false;
    }
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
                mbuf* msg_to_enq_sock = mbuf_clone(msg, core::tcp.mp);
                mbuf_pull(msg_to_enq_sock, sizeof(stcp_ip_header));
                uint16_t tcphlen  = ((tih->tcp.data_off>>4)<<2);
                mbuf_pull(msg_to_enq_sock, tcphlen);
                rxq.push(msg_to_enq_sock);

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
                mbuf_free(msg);
                throw exception("OKASHII94010");

            case TCPS_SYN_RCVD:
                mbuf_free(msg);
                throw exception("RFC MITEIGIIIII");

            default:
                mbuf_free(msg);
                throw exception("OKASHII19491: unknown state");
        }
    } else {
        mbuf_free(msg);
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
                mbuf_free(msg);
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
        core::tcp.tx_push(mbuf_clone(msg, core::tcp.mp), src);

    }
    mbuf_free(msg);
    return true;
}



void stcp_tcp_sock::print_stat(size_t rootx, size_t rooty) const
{
    screen.move(rooty, rootx);

    screen.printwln(" sock/tcp=%s/%s [this=%p]",
            sockstate2str(sock_state),
            tcpstate2str(tcp_state),
            this);

    switch (tcp_state) {
        case TCPS_LISTEN:
            screen.printwln("  - local  port: %u", ntoh16(port));
            screen.printwln("  - wait accept count: %zd", wait_accept_count);
            break;
        case TCPS_ESTABLISHED:
            screen.printwln("  - local/remote: %s:%u/%s:%u",
                    addr.c_str(), ntoh16(port), pair.c_str(), ntoh16(pair_port));
            screen.printwln("  - txq/rxq: %zd/%zd",rxq.size(), txq.size());
            screen.printwln("  - iss/irs        : %u/%u", si.iss_H(), si.irs_H());
            screen.printwln("  - snd_una        : %u", si.snd_una_H());
            screen.printwln("  - snd_nxt/rcv_nxt: %u/%u", si.snd_nxt_H(), si.rcv_nxt_H());
            screen.printwln("  - snd_win/rcv_win: %u/%u", si.snd_win_H(), si.rcv_win_H());
            screen.printwln("  - snd_wl1/wl2    : %u/%u", si.snd_wl1_H(), si.snd_wl2_H());
            break;
        case TCPS_CLOSED:
            for (size_t i=1; i<=7; i++)
                screen.printwln("                                                                ");
            break;
        default:
            break;
    }
}
} /* namespace slank */
