

#include <stcp/tcp.h>
#include <stcp/config.h>

namespace slank {



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
    move_state(STCP_TCP_ST_LISTEN);
}




void stcp_tcp_sock::move_state(tcp_socket_state next_state)
{
    DEBUG("%s to %s \n",
            tcp_socket_state2str(state),
            tcp_socket_state2str(next_state) );

    switch (state) {
        case STCP_TCP_ST_CLOSED     :
            move_state_from_CLOSED(next_state);
            break;
        case STCP_TCP_ST_LISTEN     :
            move_state_from_LISTEN(next_state);
            break;
        case STCP_TCP_ST_SYN_SENT   :
            move_state_from_SYN_SENT(next_state);
            break;
        case STCP_TCP_ST_SYN_RCVD   :
            move_state_from_SYN_RCVD(next_state);
            break;
        case STCP_TCP_ST_ESTABLISHED:
            move_state_from_ESTABLISHED(next_state);
            break;
        case STCP_TCP_ST_FIN_WAIT_1 :
            move_state_from_FIN_WAIT_1(next_state);
            break;
        case STCP_TCP_ST_FIN_WAIT_2 :
            move_state_from_FIN_WAIT_2(next_state);
            break;
        case STCP_TCP_ST_CLOSE_WAIT :
            move_state_from_CLOSE_WAIT(next_state);
            break;
        case STCP_TCP_ST_CLOSING    :
            move_state_from_CLOSING(next_state);
            break;
        case STCP_TCP_ST_LAST_ACK   :
            move_state_from_LAST_ACK(next_state);
            break;
        case STCP_TCP_ST_TIME_WAIT  :
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
        case STCP_TCP_ST_LISTEN:
        case STCP_TCP_ST_SYN_SENT:
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
        case STCP_TCP_ST_CLOSED:
        case STCP_TCP_ST_SYN_SENT:
        case STCP_TCP_ST_SYN_RCVD:
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
        case STCP_TCP_ST_CLOSED:
        case STCP_TCP_ST_SYN_RCVD:
        case STCP_TCP_ST_ESTABLISHED:
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
        case STCP_TCP_ST_ESTABLISHED:
        case STCP_TCP_ST_FIN_WAIT_1:
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
        case STCP_TCP_ST_FIN_WAIT_1:
        case STCP_TCP_ST_CLOSE_WAIT:
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
        case STCP_TCP_ST_CLOSING:
        case STCP_TCP_ST_FIN_WAIT_2:
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
        case STCP_TCP_ST_TIME_WAIT:
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
        case STCP_TCP_ST_LAST_ACK:
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
        case STCP_TCP_ST_TIME_WAIT:
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
        case STCP_TCP_ST_CLOSED:
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
        case STCP_TCP_ST_CLOSED:
            state = next_state;
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

#if 0 // for debug
    ih->print();
    th->print();
    rte::pktmbuf_dump(stdout, msg, rte::pktmbuf_pkt_len(msg));
#endif

    switch (state) {
        case STCP_TCP_ST_CLOSED:
        {
            /* reply RSTACK */
            uint16_t myport = th->dport;
            th->dport = th->sport;
            th->sport = myport;
            th->ack_num = th->seq_num + rte::bswap32(1);
            th->seq_num = 0;

            th->data_off  = sizeof(stcp_tcp_header)/4 << 4;
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

        case STCP_TCP_ST_LISTEN:
        {
            if (th->tcp_flags == STCP_TCP_FLAG_SYN) {

                /*
                 * when it recvd SYN, it reply SYNACK
                 */
                move_state(STCP_TCP_ST_SYN_RCVD);

                uint16_t myport = th->dport;
                th->dport = th->sport;
                th->sport = myport;
                th->ack_num = th->seq_num + rte::bswap32(1);
                th->seq_num = 0;

                th->data_off  = (2*sizeof(stcp_tcp_header)) << 2;
                th->tcp_flags = STCP_TCP_FLAG_SYN | STCP_TCP_FLAG_ACK;
                th->rx_win    = 0;
                th->cksum     = 0x0000;
                th->tcp_urp   = 0x0000;

                th->cksum = rte_ipv4_udptcp_cksum(
                        reinterpret_cast<ipv4_hdr*>(ih), th);

                mbuf_pull(msg, sizeof(stcp_ip_header));
                core::ip.tx_push(msg, src, STCP_IPPROTO_TCP);

                return ;
            }
            break;
        }
        case STCP_TCP_ST_SYN_RCVD:
        {
            /*
             * check packet
             */

            if (th->tcp_flags == STCP_TCP_FLAG_ACK) {

                /*
                 * when is recvd ack packet,
                 * move state to ESTABLISHED
                 */

                move_state(STCP_TCP_ST_ESTABLISHED);
            }
            break;
        }

        /*
         * TODO add behaviours each state
         */
        case STCP_TCP_ST_SYN_SENT:
        case STCP_TCP_ST_ESTABLISHED:
        case STCP_TCP_ST_FIN_WAIT_1:
        case STCP_TCP_ST_FIN_WAIT_2:
        case STCP_TCP_ST_CLOSE_WAIT:
        case STCP_TCP_ST_CLOSING:
        case STCP_TCP_ST_LAST_ACK:
        case STCP_TCP_ST_TIME_WAIT:
            throw exception("NOT IMPLEMENT YET");
            break;
        default:
            throw exception("invalid tcp sock state");
            break;
    }
}


size_t tcp_module::mss = 1460; // TODO hardcode

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

    /* Send Port Unreachable as TCP-RSTACK */
    // mbuf_push(msg, sizeof(stcp_ip_header));
    struct tcp_stream_info info;
    info.my_port   = th->dport;
    info.pair_port = th->sport;
    info.seq_num   = th->seq_num;
    info.ack_num   = th->ack_num;

    send_RSTACK(msg, src, &info);
}


void tcp_module::send_RSTACK(mbuf* msg, stcp_sockaddr_in* dst,
        tcp_stream_info* info)
{

    stcp_tcp_header* th
        = rte::pktmbuf_mtod<stcp_tcp_header*>(msg);

    // TODO very hardcode unsafe....
    stcp_ip_header* ih =
        reinterpret_cast<stcp_ip_header*>(((uint8_t*)th) - sizeof(stcp_ip_header));

    th->sport = info->my_port;
    th->dport = info->pair_port;
    th->seq_num  = 0;
    th->ack_num  = info->seq_num + rte::bswap32(1);

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
