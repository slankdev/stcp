

#include <stcp/tcp.h>
#include <stcp/config.h>

namespace slank {




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
