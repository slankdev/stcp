

#include <stcp/tcp.h>
#include <stcp/stcp.h>
#include <stcp/config.h>

namespace slank {


size_t tcp_module::mss = 1460; // TODO hardcode



void tcp_module::close_socket(stcp_tcp_sock& s) {
    for (size_t i=0; i<socks.size(); i++) {
        if (s == socks[i]) {
            socks.erase(socks.begin() + i);
            return;
        }
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
        s.write("\t%u/tcp", rte::bswap16(sock.get_port()));
    }
}


void tcp_module::rx_push(mbuf* msg, stcp_sockaddr_in* src)
{
    stcp_tcp_header* th
        = rte::pktmbuf_mtod<stcp_tcp_header*>(msg);
    rx_cnt++;

    uint16_t dst_port = th->dport;
    for (stcp_tcp_sock& sock : socks) {
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












stcp_tcp_sock& stcp_tcp_sock::socket()
{
    return core::tcp.socket();
}

void stcp_tcp_sock::close()
{
    core::tcp.close_socket(*this);
}


stcp_tcp_sock& tcp_module::socket()
{
    stcp_tcp_sock s;
    socks.push_back(s);
    return socks[socks.size()-1];
}


stcp_tcp_sock::~stcp_tcp_sock() {}


void tcp_module::proc()
{
}


} /* namespace slank */
