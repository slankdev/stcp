

#include <stcp/tcp.h>
#include <stcp/config.h>
#define UNUSED(x) (void)(x)

namespace slank {

size_t tcp_module::mss = 1460;

inline uint16_t data_length(const stcp_tcp_header* th,
        const stcp_ip_header* ih)
{
    uint16_t iptotlen = rte::bswap16(ih->total_length);
    uint16_t iphlen = (ih->version_ihl & 0x0f)<<2;
    uint16_t tcphlen  = ((th->data_off>>4)<<2);
    return iptotlen - iphlen - tcphlen;
}

inline void swap_port(stcp_tcp_header* th)
{
    uint16_t tmp = th->sport;
    th->sport    = th->dport;
    th->dport    = tmp;
}

inline bool HAS_FLAG(uint8_t flag, tcp_flags type)
{
    return ((flag & type) != 0x00);
}


stcp_tcp_sock::stcp_tcp_sock() :
                        accepted(false),
                        dead(false),
                        head(nullptr),
                        num_connected(0),
                        state(TCPS_CLOSED),
                        port(0),
                        snd_nxt(0),
                        snd_win(0),
                        iss    (0),
                        rcv_nxt(0),
                        rcv_wnd(0),
                        irs(0)
{
    DEBUG("[%p] TCP SOCK CONSTRUCTOR \n", this);
}

/*
 * Constructor for LISTEN socket
 */
stcp_tcp_sock::stcp_tcp_sock(tcpstate s, uint16_t lp, uint16_t rp,
                                    uint32_t arg_iss, uint32_t arg_irs,
                                    stcp_tcp_sock* h)
    : stcp_tcp_sock()
{
    DEBUG("[%p] TCP SOCK CONSTRUCTOR(state, lp, rp, iss, irs, head) \n", this);
    state     = s;
    port      = lp;
    pair_port = rp;
    iss       = arg_iss;
    irs       = arg_irs;
    head      = h;
}

stcp_tcp_sock::~stcp_tcp_sock()
{
    DEBUG("[%p] delete sock \n", this);
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
    DEBUG("[%p] READ datalen=%zd\n", this, rte::pktmbuf_pkt_len(m));
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
    DEBUG("[%p] accept. return new socket[%p]\n", this, sock);
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
            proc_CLOSE_WAIT();
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
        size_t data_len = rte::pktmbuf_pkt_len(msg);
        DEBUG("[%p] %s PROC send(txq.pop(), %zd)\n",
                this, tcpstate2str(state), rte::pktmbuf_pkt_len(msg));

        stcp_tcp_header* th = reinterpret_cast<stcp_tcp_header*>(
                mbuf_push(msg, sizeof(stcp_tcp_header)));
        stcp_ip_header*  ih = reinterpret_cast<stcp_ip_header*>(
                mbuf_push(msg, sizeof(stcp_ip_header)));

        /*
         * Craft IP header for tcp checksum
         */
        ih->total_length  = rte::bswap16(rte::pktmbuf_pkt_len(msg));
        ih->next_proto_id = STCP_IPPROTO_TCP;
        ih->src           = addr.sin_addr;
        ih->dst           = pair.sin_addr;

        /*
         * Craft TCP header
         */
        th->sport     = port     ;
        th->dport     = pair_port;
        th->seq_num   = rte::bswap32(snd_nxt);
        th->ack_num   = rte::bswap32(rcv_nxt);
        th->data_off  = sizeof(stcp_tcp_header) >> 2 << 4;
        th->tcp_flags = TCPF_PSH|TCPF_ACK;
        th->rx_win    = snd_win;
        th->cksum     = 0x0000;
        th->tcp_urp   = 0; // TODO hardcode

        th->cksum = rte_ipv4_udptcp_cksum(
                reinterpret_cast<ipv4_hdr*>(ih), th);

        /*
         * send to ip module
         */
        mbuf_pull(msg, sizeof(stcp_ip_header));
        core::ip.tx_push(msg, &pair, STCP_IPPROTO_TCP);

        /*
         * TODO KOKOJANAKUNE
         */
        snd_nxt += data_len;

    }
}


void stcp_tcp_sock::proc_CLOSE_WAIT()
{
    /*
     * TODO
     * These must be implemented
     */

    mbuf* msg = rte::pktmbuf_alloc(core::dpdk.get_mempool());

    /*
     * Init Mbuf
     */
    msg->pkt_len  = sizeof(stcp_tcp_header) + sizeof(stcp_ip_header);
    msg->data_len = sizeof(stcp_tcp_header) + sizeof(stcp_ip_header);
    stcp_ip_header*  ih = rte::pktmbuf_mtod<stcp_ip_header*>(msg);
    stcp_tcp_header* th = rte::pktmbuf_mtod_offset<stcp_tcp_header*>(msg,
            sizeof(stcp_ip_header));


    /*
     * Craft IP hdr for TCP-checksum
     */
    ih->src = addr.sin_addr;
    ih->dst = pair.sin_addr;
    ih->next_proto_id = 0x06;
    ih->total_length  = rte::bswap16(
            sizeof(stcp_tcp_header) + sizeof(stcp_ip_header));


    /*
     * Craft TCP FIN
     */
    th->sport     = port     ;
    th->dport     = pair_port;
    th->seq_num   = rte::bswap32(snd_nxt);
    th->ack_num   = rte::bswap32(rcv_nxt);
    th->data_off  = sizeof(stcp_tcp_header)>>2 << 4;
    th->tcp_flags = TCPF_FIN|TCPF_ACK;
    th->rx_win    = rte::bswap16(snd_win);
    th->cksum     = 0x0000;
    th->tcp_urp   = 0x0000; // TODO hardcode

    th->cksum = rte_ipv4_udptcp_cksum(
            reinterpret_cast<ipv4_hdr*>(ih), th);

    /*
     * Update Stream infos
     */
    snd_nxt++;

    /*
     * Send packet
     */
    mbuf_pull(msg, sizeof(stcp_ip_header));
    core::ip.tx_push(msg, &pair, STCP_IPPROTO_TCP);


    /*
     * Move TCP-State
     */
    move_state(TCPS_LAST_ACK);
}



void tcp_module::proc()
{
    for (size_t i=0; i<socks.size(); i++) {
        socks[i]->proc();
    }
}



void stcp_tcp_sock::proc_RST(mbuf* msg, stcp_tcp_header* th, stcp_sockaddr_in* dst)
{
    UNUSED(msg);
    UNUSED(th);
    UNUSED(dst);

    switch (state) {
        /*
         * TODO implement
         * each behaviours
         */
        case TCPS_ESTABLISHED:
        case TCPS_LISTEN:
        case TCPS_CLOSED:
        case TCPS_SYN_SENT:
        case TCPS_SYN_RCVD:
        case TCPS_FIN_WAIT_1:
        case TCPS_FIN_WAIT_2:
        case TCPS_CLOSE_WAIT:
        case TCPS_CLOSING:
        case TCPS_LAST_ACK:
        case TCPS_TIME_WAIT:
        default:
        {
            throw exception("NOT implement");
            break;
        }

    }
}


void stcp_tcp_sock::move_state_DEBUG(tcpstate next_state)
{
    DEBUG("[%p] %s -> %s (MOVE state debug) \n", this,
            tcpstate2str(state),
            tcpstate2str(next_state) );
    state = next_state;
}
void stcp_tcp_sock::bind(const struct stcp_sockaddr_in* addr, size_t addrlen)
{
    if (addrlen < sizeof(sockaddr_in))
        throw exception("Invalid addrlen");
    port = addr->sin_port;
}
void stcp_tcp_sock::listen(size_t backlog)
{
    if (backlog < 1) throw exception("OKASHII");
    num_connected = 0;
    max_connect   = backlog;
    move_state(TCPS_LISTEN);
}




void stcp_tcp_sock::move_state(tcpstate next_state)
{
    DEBUG("[%p] %s -> %s \n", this,
            tcpstate2str(state),
            tcpstate2str(next_state) );

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
    stcp_ip_header*  ih
        = rte::pktmbuf_mtod<stcp_ip_header*>(msg);
    stcp_tcp_header* th
        = rte::pktmbuf_mtod_offset<stcp_tcp_header*>(msg, sizeof(stcp_ip_header));

    {
        /*
         * TODO Proc TCP-Option
         * ERASE zeroclear tcp options
         */
        uint8_t* buf = reinterpret_cast<uint8_t*>(th);
        buf += sizeof(stcp_tcp_header);
        size_t tcpoplen = th->data_off/4 - sizeof(stcp_tcp_header);
        memset(buf, 0x00, tcpoplen);
    }

    /*
     * TODO
     * Drop or Reply RSTACK to independent packet.
     */

    switch (state) {
        case TCPS_CLOSED:
            rx_push_CLOSED(msg, src, ih, th);
            break;
        case TCPS_LISTEN:
            rx_push_LISTEN(msg, src, ih, th);
            break;
        case TCPS_SYN_RCVD:
            rx_push_SYN_RCVD(msg, src, ih, th);
            break;
        case TCPS_ESTABLISHED:
            rx_push_ESTABLISHED(msg, src, ih, th);
            break;
        case TCPS_LAST_ACK:
            rx_push_LAST_ACK(msg, src, ih, th);
            break;


        /*
         * TODO add behaviours each state
         */
        case TCPS_CLOSE_WAIT:
        case TCPS_SYN_SENT:
        case TCPS_FIN_WAIT_1:
        case TCPS_FIN_WAIT_2:
        case TCPS_CLOSING:
        case TCPS_TIME_WAIT:
            throw exception("NOT IMPLEMENT YET");
            break;
        default:
            throw exception("invalid tcp sock state");
            break;
    }
}


void stcp_tcp_sock::rx_push_CLOSED(mbuf* msg, stcp_sockaddr_in* src,
        stcp_ip_header* ih, stcp_tcp_header* th)
{
    /*
     * reply RSTACK
     */

    swap_port(th);
    th->ack_num = th->seq_num + rte::bswap32(1);
    th->seq_num = 0;

    th->tcp_flags = TCPF_RST|TCPF_ACK;
    th->rx_win    = 0;
    th->cksum     = 0x0000;
    th->tcp_urp   = 0x0000;

    th->cksum = rte_ipv4_udptcp_cksum(
            reinterpret_cast<ipv4_hdr*>(ih), th);

    mbuf_pull(msg, sizeof(stcp_ip_header));
    core::ip.tx_push(msg, src, STCP_IPPROTO_TCP);
}


void stcp_tcp_sock::rx_push_LISTEN(mbuf* msg, stcp_sockaddr_in* src,
        stcp_ip_header* ih, stcp_tcp_header* th)
{

    if (th->tcp_flags == TCPF_SYN) {

        if (max_connect <= num_connected)
            throw exception("No such space to connect");

        /*
         * Recv SYN
         *
         * Tasks
         * + Create new socket.
         * + Init stream information.
         * + Craft SYNACK packet to reply.
         * + Ctrl Mbuf and send it.
         * + Update Stream information.
         */

        /*
         * Create new socket
         */
        stcp_tcp_sock* newsock = alloc_new_sock_connected(
                TCPS_SYN_RCVD, port, th->sport,
                rte::rand() % 0xffffffff, rte::bswap32(th->seq_num), this);
        num_connected ++;
        wait_accept.push(newsock);

        newsock->addr.sin_addr = ih->dst;
        newsock->pair.sin_addr = ih->src;
        DEBUG("[%p] connect request. alloc sock [%p]\n", this, newsock);

        /*
         * Init stream information
         */
        newsock->snd_nxt = newsock->iss;
        newsock->snd_win = 512; // TODO hardcode

        newsock->rcv_nxt = newsock->irs + 1;
        newsock->rcv_wnd = rte::bswap32(th->rx_win);

        /*
         * craft SYNACK packet to reply.
         */
        swap_port(th);


        th->seq_num = rte::bswap32(newsock->snd_nxt);
        th->ack_num = rte::bswap32(newsock->rcv_nxt);

        th->rx_win    = rte::bswap16(newsock->snd_win);
        th->tcp_flags = TCPF_SYN | TCPF_ACK;
        th->cksum     = 0x0000;
        th->tcp_urp   = 0x0000; // TODO hardcode

        th->cksum = rte_ipv4_udptcp_cksum(
                reinterpret_cast<ipv4_hdr*>(ih), th);

        /*
         * Ctrl Mbuf and send it.
         * + pull mbuf
         * + send mbuf
         */
        mbuf_pull(msg, sizeof(stcp_ip_header));
        core::ip.tx_push(msg, src, STCP_IPPROTO_TCP);

        /*
         * Update stream information.
         */
        newsock->snd_nxt ++;
        return ;
    }
}
void stcp_tcp_sock::rx_push_SYN_RCVD(mbuf* msg, stcp_sockaddr_in* src,
        stcp_ip_header* ih, stcp_tcp_header* th)
{
    UNUSED(msg);
    UNUSED(src);
    UNUSED(ih);

    /*
     * check packet is this stream's one.
     */
    if (rte::bswap32(th->seq_num) != rcv_nxt) {
        DEBUG("[%p] invalid sequence number seg=%u(0x%x), sock=%u(0x%x)\n", this,
                rte::bswap32(th->seq_num), rte::bswap32(th->seq_num),
                rcv_nxt, rcv_nxt);
        return;
    }
    if (rte::bswap32(th->ack_num) != snd_nxt) {
        DEBUG("[%p] invalid acknouledge number \n", this);
        return;
    }

    if (HAS_FLAG(th->tcp_flags, TCPF_ACK)) {
        /*
         * when recvd packet is ACK,
         * move state to ESTABLISHED
         */
        move_state(TCPS_ESTABLISHED);
    } else {
        DEBUG("[%p] Unexpected packet \n", this);
    }
}


void stcp_tcp_sock::rx_push_ESTABLISHED(mbuf* msg, stcp_sockaddr_in* src,
        stcp_ip_header* ih, stcp_tcp_header* th)
{
    uint16_t tcpdlen = data_length(th, ih);

    /*
     * TODO ERASE move implementation location
     * The code that checks msg is RST need to
     * move implementation location to it that
     * should implement location.
     */


    /*
     * Operate RST packet
     */
    if (HAS_FLAG(th->tcp_flags, TCPF_RST)) {
        proc_RST(msg, th, src);
        return;
    }


    if (HAS_FLAG(th->tcp_flags, TCPF_PSH) &&
            HAS_FLAG(th->tcp_flags, TCPF_ACK)) {
        DEBUG("[%p] ESTABLISHED RCV DATA with PSHACK\n", this);

        /*
         * Recved PSHACK+data
         * Reply ACK
         *
         * Tasks
         *  + Update Stream infos
         *  + Craft packet to reply
         *  + Update mbuf data length.
         *  + Ctrl mbuf and send it.
         */

        /*
         * Update Stream infos
         */
        snd_nxt  = rte::bswap32(th->ack_num);
        rcv_nxt  = rte::bswap32(th->seq_num) + tcpdlen;

        /*
         * Craft ACK-packet to reply
         */
        swap_port(th);
        th->rx_win  = rte::bswap16(snd_win);
        th->tcp_flags = TCPF_ACK;
        th->seq_num = rte::bswap32(snd_nxt);
        th->ack_num = rte::bswap32(rcv_nxt);
        th->cksum     = 0x0000;
        th->tcp_urp   = 0x0000; // TODO hardcode

        ih->total_length = rte::bswap16(
                sizeof(stcp_tcp_header) +
                sizeof(stcp_ip_header));
        th->cksum = rte_ipv4_udptcp_cksum(
                reinterpret_cast<ipv4_hdr*>(ih), th);

        /*
         * Copy mbuf to socket rxq
         */
        mbuf* sock_rxq_msg = rte::pktmbuf_clone(msg, core::dpdk.get_mempool());
        size_t tcp_hlen = (th->data_off >> 2);
        size_t ip_hlen  = ((ih->version_ihl&0xf) << 2);
        rte::pktmbuf_adj(sock_rxq_msg, tcp_hlen+ip_hlen);
        rxq.push(sock_rxq_msg);

        /*
         * Update mbuf data length
         */
        rte::pktmbuf_trim(msg, tcpdlen);

        /*
         * Ctrl mbuf and Send it.
         */
        mbuf_pull(msg, sizeof(stcp_ip_header));
        core::ip.tx_push(msg, src, STCP_IPPROTO_TCP);

    } else if (HAS_FLAG(th->tcp_flags, TCPF_FIN)) {

        /*
         * Update Stream infos
         */
        snd_nxt = rte::bswap32(th->ack_num);
        rcv_nxt = rte::bswap32(th->seq_num) + tcpdlen;
        rcv_nxt ++;

        /*
         * Recv FIN
         * Send ACK
         */

        swap_port(th);
        th->tcp_flags = TCPF_ACK;
        th->seq_num = rte::bswap32(snd_nxt);
        th->ack_num = rte::bswap32(rcv_nxt);
        th->rx_win  = rte::bswap16(snd_win);
        th->cksum   = 0x0000;
        th->tcp_urp = 0x0000; // TODO hardcode

        ih->total_length = rte::bswap16(
                sizeof(stcp_tcp_header) +
                sizeof(stcp_ip_header));
        th->cksum = rte_ipv4_udptcp_cksum(
                reinterpret_cast<ipv4_hdr*>(ih), th);

        mbuf_pull(msg, sizeof(stcp_ip_header));
        core::ip.tx_push(msg, src, STCP_IPPROTO_TCP);

        core::get_myip(&addr.sin_addr, 0); // TODO port number hardcode
        pair = *src;
        pair_port = th->dport;
        move_state(TCPS_CLOSE_WAIT);

    } else if (HAS_FLAG(th->tcp_flags, TCPF_ACK)) {
        // MARKED
        DEBUG("[%p] PROC send(,%u) success\n", this, rte::bswap32(th->ack_num) - snd_nxt);
        snd_nxt = rte::bswap32(th->ack_num);
    } else {
        std::string errstr;
        errstr.resize(256);
        sprintf(&errstr[0], "[%p] independent", this);
        errstr.resize(strlen(&errstr[0]));
        throw exception(errstr.c_str());
    }

}

void stcp_tcp_sock::rx_push_LAST_ACK(mbuf* msg, stcp_sockaddr_in* src,
                                    stcp_ip_header* ih, stcp_tcp_header* th)
{
    UNUSED(ih);
    UNUSED(src);
    UNUSED(msg);
    if (HAS_FLAG(th->tcp_flags, TCPF_ACK)) {
        move_state(TCPS_CLOSED);
    }
}



void stcp_tcp_sock::print_stat() const
{
    stat& s = stat::instance();
    s.write("\t%u/tcp state=%s[this=%p] rx/tx=%zd/%zd %u/%u %u",
            rte::bswap16(port),
            tcpstate2str(state), this,
            rxq.size(), txq.size(),
            snd_nxt, rcv_nxt,
            rte::bswap16(pair_port));

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
                    rte::bswap16(port),
                    rte::bswap16(pair_port));
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
            tcpstate s = sock->state;
            if (s==TCPS_CLOSED
                    || s==TCPS_CLOSE_WAIT
                    || s==TCPS_TIME_WAIT ) {
                continue;
            }


            mbuf* m = rte::pktmbuf_clone(msg, core::dpdk.get_mempool());
            mbuf_push(m, sizeof(stcp_ip_header));
            sock->rx_push(m, src);
            find_socket = true;
        }
    }

    if (!find_socket) {
        /*
         * Send Port Unreachable as TCP-RSTACK
         */
        send_RSTACK(msg, src);
    }
    rte::pktmbuf_free(msg);
}


void tcp_module::send_RSTACK(mbuf* msg, stcp_sockaddr_in* dst)
{
    stcp_ip_header* ih = reinterpret_cast<stcp_ip_header*>(
            mbuf_push(msg, sizeof(stcp_ip_header)));
    stcp_tcp_header* th
        = rte::pktmbuf_mtod_offset<stcp_tcp_header*>(msg, sizeof(stcp_tcp_header));

    /*
     * Delete TCP Option field
     */
    size_t optionlen =
        rte::pktmbuf_pkt_len(msg) - sizeof(stcp_ip_header) - sizeof(stcp_tcp_header);
    rte::pktmbuf_trim(msg, optionlen);


    /*
     * Set IP header for TCP checksum
     */
    ih->src           = ih->dst;
    ih->dst           = dst->sin_addr;
    ih->next_proto_id = STCP_IPPROTO_TCP;
    ih->total_length  = rte::bswap16(rte::pktmbuf_pkt_len(msg));

    /*
     * Set TCP header
     *
     */
    swap_port(th);
    th->ack_num  = th->seq_num + rte::bswap32(1);
    th->seq_num  = 0;

    th->data_off = sizeof(stcp_tcp_header)/4 << 4;
    th->tcp_flags    = TCPF_RST|TCPF_ACK;
    th->rx_win   = 0;
    th->cksum    = 0x0000;
    th->tcp_urp  = 0x0000;

    th->cksum = rte_ipv4_udptcp_cksum(
            reinterpret_cast<ipv4_hdr*>(ih), th);

    mbuf_pull(msg, sizeof(stcp_ip_header));
    core::ip.tx_push(msg, dst, STCP_IPPROTO_TCP);
}






} /* namespace slank */
