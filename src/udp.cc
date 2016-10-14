

#include <stcp/udp.h>
#include <stcp/stcp.h>


namespace slank {


void udp_module::rx_push(mbuf* msg, const stcp_sockaddr* src)
{
    stcp_udp_header* uh
        = rte::pktmbuf_mtod<stcp_udp_header*>(msg);

    uint16_t dst_port = uh->dport;
    for (udp_sock& sock : socks) {
        if (sock.port == dst_port) {
            sock_data d(msg, *src);
            sock.rxq.push(d);
        }
    }
    core::instance().icmp.send_err(STCP_ICMP_UNREACH, STCP_ICMP_UNREACH_PORT, src, msg);
}


} /* namespace slank */
