

#include <stcp/tcp.h>
#include <stcp/config.h>

namespace slank {




void stcp_tcp_sock::move_state(tcp_socket_state next_state)
{
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
    for (const auto sock : socks) {
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
    for (auto sock : socks) {
        src->sin_port = th->sport;
        if (sock.get_port() == dst_port) {
            throw exception("NOT IMPLE");
            // mbuf_pull(msg, sizeof(stcp_tcp_header)); // TODO tcp hlen hardcode
            // TODO
            // sock.rx_data_push(d);
            // return ;
        }
    }

    /* Send ICMP Port Unreachable  */
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
    th->cksum    = 0x0000; // TODO
    th->tcp_urp  = 0x0000;

    th->cksum = rte_ipv4_udptcp_cksum(
            reinterpret_cast<ipv4_hdr*>(ih), th);

    core::ip.tx_push(msg, dst, STCP_IPPROTO_TCP);
}






} /* namespace slank */
