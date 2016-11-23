

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
                        state(STCP_TCPS_CLOSED), port(0),
                        snd_una(0), snd_nxt(0), snd_win(0), snd_up (0),
                        snd_wl1(0), snd_wl2(0), iss    (0),
                        rcv_nxt(0), rcv_wnd(0), rcv_up (0), irs(0)
{
    DEBUG("[%p] alloc sock \n", this);
}

stcp_tcp_sock::~stcp_tcp_sock()
{
    DEBUG("[%p] delete sock \n", this);
    if (head) {
        head->num_connected --;
    }
}


void stcp_tcp_sock::write(mbuf* msg)
{
    if (state != STCP_TCPS_ESTABLISHED) {
        std::string errstr = "Not Open Port state=";
        errstr += tcp_socket_state2str(state);
        throw exception(errstr.c_str());
    }
    txq.push(msg);
}


mbuf* stcp_tcp_sock::read()
{
    while (rxq.size() == 0) {
        if (state == STCP_TCPS_CLOSED) {
            std::string errstr = "Not Open Port state=";
            errstr += tcp_socket_state2str(state);
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
        case STCP_TCPS_CLOSE_WAIT:
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
            th->tcp_flags = STCP_TCP_FLAG_FIN|STCP_TCP_FLAG_ACK;
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
            move_state(STCP_TCPS_LAST_ACK);
            break;
        }

        case STCP_TCPS_ESTABLISHED:
        {
            while (!txq.empty()) {
                mbuf* msg = txq.pop();
                size_t data_len = rte::pktmbuf_pkt_len(msg);
                DEBUG("[%p] %s PROC send(txq.pop(), %zd)\n",
                        this, tcp_socket_state2str(state), rte::pktmbuf_pkt_len(msg));

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
                th->tcp_flags = STCP_TCP_FLAG_PSH|STCP_TCP_FLAG_ACK;
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
            break;
        }
        case STCP_TCPS_LISTEN:
        case STCP_TCPS_CLOSED:
        case STCP_TCPS_SYN_SENT:
        case STCP_TCPS_SYN_RCVD:
        case STCP_TCPS_FIN_WAIT_1:
        case STCP_TCPS_FIN_WAIT_2:
        case STCP_TCPS_CLOSING:
        case STCP_TCPS_LAST_ACK:
        case STCP_TCPS_TIME_WAIT:
        {
            /*
             * TODO
             * Not Implement yet.
             * No Operation
             */
            break;
        }
        default:
        {
            throw exception("UNKNOWN TCP STATE!!!");
            break;
        }
    }
}


void tcp_module::proc()
{
    for (size_t i=0; i<socks.size(); i++) {
        socks[i]->proc();
    }
}



void stcp_tcp_sock::do_RST(stcp_tcp_header* th)
{

#define UNUSED(x) (void)(x)
    UNUSED(th); // TODO ERASE

    switch (state) {
        case STCP_TCPS_ESTABLISHED:
        {
            /* nop */
            break;
        }


        /*
         * TODO implement
         * each behaviours
         */
        case STCP_TCPS_LISTEN:
        case STCP_TCPS_CLOSED:
        case STCP_TCPS_SYN_SENT:
        case STCP_TCPS_SYN_RCVD:
        case STCP_TCPS_FIN_WAIT_1:
        case STCP_TCPS_FIN_WAIT_2:
        case STCP_TCPS_CLOSE_WAIT:
        case STCP_TCPS_CLOSING:
        case STCP_TCPS_LAST_ACK:
        case STCP_TCPS_TIME_WAIT:
        default:
        {
            throw exception("NOT implement");
            break;
        }

    }
}
void stcp_tcp_sock::move_state_DEBUG(tcp_socket_state next_state)
{
    DEBUG("[%p] %s -> %s (MOVE state debug) \n", this,
            tcp_socket_state2str(state),
            tcp_socket_state2str(next_state) );
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
    move_state(STCP_TCPS_LISTEN);
}




void stcp_tcp_sock::move_state(tcp_socket_state next_state)
{
    DEBUG("[%p] %s -> %s \n", this,
            tcp_socket_state2str(state),
            tcp_socket_state2str(next_state) );

    switch (state) {
        case STCP_TCPS_CLOSED     :
            move_state_from_CLOSED(next_state);
            break;
        case STCP_TCPS_LISTEN     :
            move_state_from_LISTEN(next_state);
            break;
        case STCP_TCPS_SYN_SENT   :
            move_state_from_SYN_SENT(next_state);
            break;
        case STCP_TCPS_SYN_RCVD   :
            move_state_from_SYN_RCVD(next_state);
            break;
        case STCP_TCPS_ESTABLISHED:
            move_state_from_ESTABLISHED(next_state);
            break;
        case STCP_TCPS_FIN_WAIT_1 :
            move_state_from_FIN_WAIT_1(next_state);
            break;
        case STCP_TCPS_FIN_WAIT_2 :
            move_state_from_FIN_WAIT_2(next_state);
            break;
        case STCP_TCPS_CLOSE_WAIT :
            move_state_from_CLOSE_WAIT(next_state);
            break;
        case STCP_TCPS_CLOSING    :
            move_state_from_CLOSING(next_state);
            break;
        case STCP_TCPS_LAST_ACK   :
            move_state_from_LAST_ACK(next_state);
            break;
        case STCP_TCPS_TIME_WAIT  :
            move_state_from_TIME_WAIT(next_state);
            break;
        default:
            throw exception("invalid tcp sock state");
            break;
    }
}


void stcp_tcp_sock::move_state_from_CLOSED(tcp_socket_state next_state)
{
    switch (next_state) {
        case STCP_TCPS_LISTEN:
        case STCP_TCPS_SYN_SENT:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_LISTEN(tcp_socket_state next_state)
{
    switch (next_state) {
        case STCP_TCPS_CLOSED:
        case STCP_TCPS_SYN_SENT:
        case STCP_TCPS_SYN_RCVD:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_SYN_SENT(tcp_socket_state next_state)
{
    switch (next_state) {
        case STCP_TCPS_CLOSED:
        case STCP_TCPS_SYN_RCVD:
        case STCP_TCPS_ESTABLISHED:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_SYN_RCVD(tcp_socket_state next_state)
{
    switch (next_state) {
        case STCP_TCPS_ESTABLISHED:
        case STCP_TCPS_FIN_WAIT_1:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_ESTABLISHED(tcp_socket_state next_state)
{
    switch (next_state) {
        case STCP_TCPS_FIN_WAIT_1:
        case STCP_TCPS_CLOSE_WAIT:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_FIN_WAIT_1(tcp_socket_state next_state)
{
    switch (next_state) {
        case STCP_TCPS_CLOSING:
        case STCP_TCPS_FIN_WAIT_2:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_FIN_WAIT_2(tcp_socket_state next_state)
{
    switch (next_state) {
        case STCP_TCPS_TIME_WAIT:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_CLOSE_WAIT(tcp_socket_state next_state)
{
    switch (next_state) {
        case STCP_TCPS_LAST_ACK:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_CLOSING(tcp_socket_state next_state)
{
    switch (next_state) {
        case STCP_TCPS_TIME_WAIT:
            state = next_state;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_LAST_ACK(tcp_socket_state next_state)
{
    switch (next_state) {
        case STCP_TCPS_CLOSED:
            state = next_state;
            dead = true;
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}
void stcp_tcp_sock::move_state_from_TIME_WAIT(tcp_socket_state next_state)
{
    switch (next_state) {
        case STCP_TCPS_CLOSED:
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
        case STCP_TCPS_CLOSED:
            rx_push_CLOSED(msg, src, ih, th);
            break;
        case STCP_TCPS_LISTEN:
            rx_push_LISTEN(msg, src, ih, th);
            break;
        case STCP_TCPS_SYN_RCVD:
            rx_push_SYN_RCVD(msg, src, ih, th);
            break;
        case STCP_TCPS_ESTABLISHED:
            rx_push_ESTABLISHED(msg, src, ih, th);
            break;
        case STCP_TCPS_LAST_ACK:
            rx_push_LAST_ACK(msg, src, ih, th);
            break;


        /*
         * TODO add behaviours each state
         */
        case STCP_TCPS_CLOSE_WAIT:
        case STCP_TCPS_SYN_SENT:
        case STCP_TCPS_FIN_WAIT_1:
        case STCP_TCPS_FIN_WAIT_2:
        case STCP_TCPS_CLOSING:
        case STCP_TCPS_TIME_WAIT:
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

    th->tcp_flags = STCP_TCP_FLAG_RST|STCP_TCP_FLAG_ACK;
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
    currend_seg cseg(th, ih);

    if (th->tcp_flags == STCP_TCP_FLAG_SYN) {

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
        stcp_tcp_sock* newsock = core::create_tcp_socket();
        num_connected ++;
        newsock->port = port;
        newsock->pair_port = th->sport;
        newsock->addr.sin_addr = ih->dst;
        newsock->pair.sin_addr = ih->src;
        newsock->state = STCP_TCPS_SYN_RCVD;
        DEBUG("[%p] connect request. alloc sock [%p]\n", this, newsock);

        /*
         * Link Slave-Socket
         */
        newsock->head = this;
        wait_accept.push(newsock);

        /*
         * Init stream information
         */
        newsock->iss = (rte::rand() % 0xffffffff); // TODO hardcode
        newsock->snd_una = 0;
        newsock->snd_nxt = newsock->iss;
        newsock->snd_win = 512; // TODO hardcode
        newsock->snd_up  = 0;
        newsock->snd_wl1 = cseg.seg_seq;
        newsock->snd_wl2 = cseg.seg_ack;

        newsock->irs = cseg.seg_seq;
        newsock->rcv_nxt = newsock->irs + 1;
        newsock->rcv_wnd = cseg.seg_wnd;
        newsock->rcv_up  = 0;

        /*
         * craft SYNACK packet to reply.
         */
        swap_port(th);


        th->seq_num = rte::bswap32(newsock->snd_nxt);
        th->ack_num = rte::bswap32(newsock->rcv_nxt);

        th->rx_win    = rte::bswap16(newsock->snd_win);
        th->tcp_flags = STCP_TCP_FLAG_SYN | STCP_TCP_FLAG_ACK;
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
    currend_seg cseg(th, ih);
    /*
     * check packet is this stream's one.
     */
    if (cseg.seg_seq != rcv_nxt) {
        DEBUG("[%p] invalid sequence number seg=%u(0x%x), sock=%u(0x%x)\n", this,
                cseg.seg_seq, cseg.seg_seq,
                rcv_nxt, rcv_nxt);
        return;
    }
    if (cseg.seg_ack != snd_nxt) {
        DEBUG("[%p] invalid acknouledge number \n", this);
        return;
    }

    if (HAS_FLAG(th->tcp_flags, STCP_TCP_FLAG_ACK)) {
        /*
         * when recvd packet is ACK,
         * move state to ESTABLISHED
         */
        move_state(STCP_TCPS_ESTABLISHED);
    } else {
        DEBUG("[%p] Unexpected packet \n", this);
    }
}


void stcp_tcp_sock::rx_push_ESTABLISHED(mbuf* msg, stcp_sockaddr_in* src,
        stcp_ip_header* ih, stcp_tcp_header* th)
{
    uint16_t tcpdlen = data_length(th, ih);
    currend_seg cseg(th, ih);
    /*
     * TODO ERASE move implementation location
     * The code that checks msg is RST need to
     * move implementation location to it that
     * should implement location.
     */

#if 0
    // if ((th->tcp_flags & STCP_TCP_FLAG_RST) != 0x00) {
    //     do_RST(th);
    //     rte::pktmbuf_free(msg);
    //     return;
    // }
#endif

#if 0
    /*
     * TODO
     * filter packet as TCP-SEG to socket-queue
     */
    if (th->seq_num == rcv_nxt) {
        DEBUG("SLNAKDEVSLANKDV: TIGAU1\n");
    }
    if (th->ack_num == snd_nxt) {
        DEBUG("SLNAKDEVSLANKDV: TIGAU2\n");
    }
#endif


    if (HAS_FLAG(th->tcp_flags, STCP_TCP_FLAG_PSH) &&
            HAS_FLAG(th->tcp_flags, STCP_TCP_FLAG_ACK)) {
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
        snd_nxt  = cseg.seg_ack;
        rcv_nxt  = cseg.seg_seq + tcpdlen;

        /*
         * Craft ACK-packet to reply
         */
        swap_port(th);
        th->rx_win  = rte::bswap16(snd_win);
        th->tcp_flags = STCP_TCP_FLAG_ACK;
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

    } else if (HAS_FLAG(th->tcp_flags, STCP_TCP_FLAG_FIN)) {

        /*
         * Update Stream infos
         */
        snd_nxt = cseg.seg_ack;
        rcv_nxt = cseg.seg_seq + tcpdlen;
        rcv_nxt ++;

        /*
         * Recv FIN
         * Send ACK
         */

        swap_port(th);
        th->tcp_flags = STCP_TCP_FLAG_ACK;
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
        move_state(STCP_TCPS_CLOSE_WAIT);

    } else if (HAS_FLAG(th->tcp_flags, STCP_TCP_FLAG_ACK)) {

        // #<{(|
        //  * TODO
        //  * filter packet as TCP-SEG to socket-queue
        //  |)}>#
        // if (th->seq_num == rcv_nxt) {
        //     DEBUG("SLNAKDEVSLANKDV: TIGAU1\n");
        // }
        // if (th->ack_num == snd_nxt) {
        //     DEBUG("SLNAKDEVSLANKDV: TIGAU2\n");
        // }

        // TODO
        // DEBUG("[%p] PROC send(,%u) success\n", this, rte::bswap32(th->ack_num) - snd_nxt);
        // snd_nxt = rte::bswap32(th->ack_num);
    } else {
        DEBUG("[%p] independent packet \n", this);
    }

}

void stcp_tcp_sock::rx_push_LAST_ACK(mbuf* msg, stcp_sockaddr_in* src,
                                    stcp_ip_header* ih, stcp_tcp_header* th)
{
    UNUSED(ih);
    UNUSED(src);
    UNUSED(msg);
    if (HAS_FLAG(th->tcp_flags, STCP_TCP_FLAG_ACK)) {
        move_state(STCP_TCPS_CLOSED);
    }
}



void stcp_tcp_sock::print_stat() const
{
    stat& s = stat::instance();
    s.write("\t%u/tcp state=%s[this=%p] rx/tx=%zd/%zd %u/%u %u",
            rte::bswap16(port),
            tcp_socket_state2str(state), this,
            rxq.size(), txq.size(),
            snd_nxt, rcv_nxt,
            rte::bswap16(pair_port));

#if 0
    switch (state) {
#if 0
        case STCP_TCPS_LISTEN:
            s.write("\t - socket alloced %zd/%zd", num_connected, max_connect);
            s.write("\n\n\n");
            break;
        case STCP_TCPS_CLOSED:
            s.write("\n\n\n");
            break;
        default:
            s.write("\n\n\n");
            break;
#endif
        case STCP_TCPS_LISTEN:
            s.write("\t - socket alloced %zd/%zd", num_connected, max_connect);
            break;
        case STCP_TCPS_ESTABLISHED:
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
            tcp_socket_state s = sock->state;
            if (s==STCP_TCPS_CLOSED
                    || s==STCP_TCPS_CLOSE_WAIT
                    || s==STCP_TCPS_TIME_WAIT ) {
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
    ih->print();

    /*
     * Set TCP header
     *
     */
    swap_port(th);
    th->ack_num  = th->seq_num + rte::bswap32(1);
    th->seq_num  = 0;

    th->data_off = sizeof(stcp_tcp_header)/4 << 4;
    th->tcp_flags    = STCP_TCP_FLAG_RST|STCP_TCP_FLAG_ACK;
    th->rx_win   = 0;
    th->cksum    = 0x0000;
    th->tcp_urp  = 0x0000;

    th->cksum = rte_ipv4_udptcp_cksum(
            reinterpret_cast<ipv4_hdr*>(ih), th);

    mbuf_pull(msg, sizeof(stcp_ip_header));
    core::ip.tx_push(msg, dst, STCP_IPPROTO_TCP);
}






} /* namespace slank */
