

#include <stcp/udp.h>
#include <stcp/stcp.h>


namespace slank {


void udp_module::rx_push(mbuf* msg, const stcp_sockaddr* src)
{
    DEBUG("ICMP rx_push from %s\n", src->c_str());

    // stcp_udp_header* uh
    //     = rte::pktmbuf_mtod<stcp_udp_header*>(msg);
    //
    // uint16_t dst_port = uh->dport;
    // for (stcp_udp_sock& sock : socks) {
    //     if (sock.port == dst_port) {
    //         stcp_udp_sock_data d(msg, *src);
    //         sock.rxq.push(d);
    //     }
    // }

    DEBUG("\tICMP rx_push: port unreach\n");
    mbuf_push(msg, sizeof(stcp_ip_header));
    core::instance().icmp.send_err(STCP_ICMP_UNREACH, STCP_ICMP_UNREACH_PORT, src, msg);
}


} /* namespace slank */
