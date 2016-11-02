

#include <stcp/tcp.h>
#include <stcp/config.h>

namespace slank {

size_t tcp_module::mss = 1460;



void stcp_tcp_sock::proc()
{
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

        case STCP_TCPS_LISTEN:
        case STCP_TCPS_CLOSED:
        case STCP_TCPS_SYN_SENT:
        case STCP_TCPS_SYN_RCVD:
        case STCP_TCPS_ESTABLISHED:
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
    for (stcp_tcp_sock& sock : socks) {
        sock.proc();
    }
}


void stcp_tcp_sock::check_RST(stcp_tcp_header* th)
{
    if ((th->tcp_flags & STCP_TCP_FLAG_RST) != 0x00) {
        move_state_DEBUG(STCP_TCPS_LISTEN);
    }
}
void stcp_tcp_sock::move_state_DEBUG(tcp_socket_state next_state)
{
    DEBUG("%s -> %s (MOVE state debug) \n",
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
    connections.resize(backlog);
    move_state(STCP_TCPS_LISTEN);
}




void stcp_tcp_sock::move_state(tcp_socket_state next_state)
{
    DEBUG("%s -> %s \n",
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
            break;
        default:
            throw exception("invalid state-change");
            break;
    }
}


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





/*
 * msg: points ip_header
 */
void stcp_tcp_sock::rx_push(mbuf* msg,stcp_sockaddr_in* src)
{
    stcp_ip_header*  ih
        = rte::pktmbuf_mtod<stcp_ip_header*>(msg);
    stcp_tcp_header* th
        = rte::pktmbuf_mtod_offset<stcp_tcp_header*>(msg, sizeof(stcp_ip_header));
    currend_seg cseg(th, ih);
    uint16_t tcpdlen = data_length(th, ih);

    /*
     * TODO Proc TCP-Option
     */

    /* TODO ERASE zeroclear tcp options */
    {
        /*
         * Zero Clear at TCP option field
         */
        uint8_t* buf = reinterpret_cast<uint8_t*>(th);
        buf += sizeof(stcp_tcp_header);
        size_t tcpoplen = th->data_off/4 - sizeof(stcp_tcp_header);
        // if (tcpoplen > 0) {
        //     DEBUG("clear opt %zd byte\n", tcpoplen);
        // } else {
        //     DEBUG("no option \n");
        // }
        memset(buf, 0x00, tcpoplen);
    }


    /*
     * TODO
     * Drop or Reply RSTACK to independent packet.
     */

    /*
     * TODO
     * Enqueue packet as TCP-SEG to socket-queue
     */


    /*
     * TODO move implementation in socket::proc()
     */
    switch (state) {
        case STCP_TCPS_CLOSED:
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
            break;
        }
        case STCP_TCPS_LISTEN:
        {
            if (th->tcp_flags == STCP_TCP_FLAG_SYN) {

                /*
                 * Recv SYN
                 *
                 * Tasks
                 * + Init stream information.
                 * + Craft SYNACK packet to reply.
                 * + Ctrl Mbuf and send it.
                 * + Update Stream information.
                 */
                iss = 123456700; // TODO hardcode
                snd_una = 0;
                snd_nxt = iss;
                snd_win = 512; // TODO hardcode
                snd_up  = 0;
                snd_wl1 = cseg.seg_seq;
                snd_wl2 = cseg.seg_ack;


                irs = cseg.seg_seq;
                rcv_nxt = irs+1;
                rcv_wnd = cseg.seg_wnd;
                rcv_up  = 0;

                /*
                 * craft SYNACK packet to reply.
                 */
                swap_port(th);
                th->seq_num = rte::bswap32(snd_nxt);
                th->ack_num = rte::bswap32(rcv_nxt);

                th->rx_win    = rte::bswap16(snd_win);
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
                move_state(STCP_TCPS_SYN_RCVD);

                /*
                 * Update stream information.
                 */
                snd_nxt ++;
                return ;
            }
            break;
        }
        case STCP_TCPS_SYN_RCVD:
        {
            /*
             * check packet is this stream's one.
             */
            if (cseg.seg_seq != rcv_nxt) {
                DEBUG("invalid sequence number \n");
                return;
            }
            if (cseg.seg_ack != snd_nxt) {
                DEBUG("invalid acknouledge number \n");
                return;
            }

            if (th->tcp_flags == STCP_TCP_FLAG_ACK) {
                /*
                 * when recvd packet is ACK,
                 * move state to ESTABLISHED
                 */
                move_state(STCP_TCPS_ESTABLISHED);
            } else {
                DEBUG("Unexpected packet \n");
            }
            break;
        }
        case STCP_TCPS_ESTABLISHED:
        {
            /*
             * TODO ERASE move implementation location
             * The code that checks msg is RST need to
             * move implementation location to it that
             * should implement location.
             */
            check_RST(th);

            if ((th->tcp_flags&STCP_TCP_FLAG_PSH) != 0x00 &&
                    (th->tcp_flags&STCP_TCP_FLAG_ACK) != 0x00) {
                DEBUG("ESTABLISHED: recv PSHACK \n");

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
                 * Update mbuf data length
                 */
                rte::pktmbuf_trim(msg, tcpdlen);

                /*
                 * Ctrl mbuf and Send it.
                 */
                mbuf_pull(msg, sizeof(stcp_ip_header));
                core::ip.tx_push(msg, src, STCP_IPPROTO_TCP);

            } else if ((th->tcp_flags&STCP_TCP_FLAG_FIN) != 0x00) {

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

            } else {
                DEBUG("independent packet \n");
            }

            break;
        }
        case STCP_TCPS_LAST_ACK:
        {
            if ((th->tcp_flags&STCP_TCP_FLAG_ACK) != 0x00) {
                move_state(STCP_TCPS_CLOSED);
            }
            break;
        }


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



void tcp_module::print_stat() const
{
    stat& s = stat::instance();
    s.write("TCP module");
    s.write("\tRX Packets %zd", rx_cnt);
    s.write("\tTX Packets %zd", tx_cnt);

    if (socks.size() > 0) {
        s.write("");
        s.write("\tNetStat %zd ports", socks.size());
    }
    for (const stcp_tcp_sock& sock : socks) {
        s.write("\t%u/tcp state=%s", rte::bswap16(sock.get_port()),
                tcp_socket_state2str(sock.get_state()));
    }
}


void tcp_module::rx_push(mbuf* msg, stcp_sockaddr_in* src)
{
    stcp_tcp_header* th
        = rte::pktmbuf_mtod<stcp_tcp_header*>(msg);
    rx_cnt++;

    uint16_t dst_port = th->dport;
    for (stcp_tcp_sock& sock : socks) {
        if (sock.get_port() == dst_port) {
            mbuf_push(msg, sizeof(stcp_ip_header));
            sock.rx_push(msg, src);
            return;
        }
    }

    /*
     * Send Port Unreachable as TCP-RSTACK
     */
    send_RSTACK(msg, src);
}


void tcp_module::send_RSTACK(mbuf* msg, stcp_sockaddr_in* dst)
{

    stcp_tcp_header* th
        = rte::pktmbuf_mtod<stcp_tcp_header*>(msg);

    // TODO very hardcode unsafe....
    stcp_ip_header* ih =
        reinterpret_cast<stcp_ip_header*>(((uint8_t*)th) - sizeof(stcp_ip_header));

    swap_port(th);
    th->seq_num  = 0;
    th->ack_num  = th->seq_num + rte::bswap32(1);

    th->data_off = sizeof(stcp_tcp_header)/4 << 4;
    th->tcp_flags    = STCP_TCP_FLAG_RST|STCP_TCP_FLAG_ACK;
    th->rx_win   = 0;
    th->cksum    = 0x0000;
    th->tcp_urp  = 0x0000;

    th->cksum = rte_ipv4_udptcp_cksum(
            reinterpret_cast<ipv4_hdr*>(ih), th);

    core::ip.tx_push(msg, dst, STCP_IPPROTO_TCP);
}






} /* namespace slank */
