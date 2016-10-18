

#include <stcp/udp.h>
#include <stcp/stcp.h>
#include <stcp/config.h>


namespace slank {


bool stcp_udp_sock::recvfrom(mbuf** msg, stcp_sockaddr* src)
{
    if (!binded) 
        throw slankdev::exception("socket is not binded");

    if (rxq.size() > 0) {
        stcp_udp_sockdata d = rxq.front();
        rxq.pop();

        stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(src);
        *src = d.addr;
        sin->sin_port = d.sport;

        *msg = d.msg;
        return true;
    } else {
        return false;
    }
}



void stcp_udp_sock::sendto(mbuf* msg, const stcp_sockaddr* dst)
{
    const stcp_sockaddr_in* sin 
        = reinterpret_cast<const stcp_sockaddr_in*>(dst);

    stcp_sockaddr s(STCP_AF_INET);
    s = *dst;

    core::instance().udp.tx_push(msg, dst,
            rte::bswap16(addr.sin_port), 
            sin->sin_port);
}

void udp_module::tx_push(mbuf* msg, 
        const stcp_sockaddr* dst, uint16_t srcp, uint16_t dstp)
{
    uint16_t udplen = rte::pktmbuf_pkt_len(msg);

    stcp_udp_header* uh = 
        reinterpret_cast<stcp_udp_header*>(mbuf_push(msg, sizeof(stcp_udp_header)));
    uh->sport = rte::bswap16(srcp);
    uh->dport = rte::bswap16(dstp);
    uh->len   = rte::bswap16(sizeof(stcp_udp_header) + udplen);
    uh->cksum = 0x0000;

    core::instance().ip.tx_push(msg, dst, STCP_IPPROTO_UDP);

}

void udp_module::rx_push(mbuf* msg, const stcp_sockaddr* src)
{
    stcp_udp_header* uh
        = rte::pktmbuf_mtod<stcp_udp_header*>(msg);

    uint16_t dst_port = uh->dport;
    // DEBUG("ICMP rx_push from %s to port%u \n", src->c_str(), dst_port);

    for (stcp_udp_sock& sock : socks) {
        if (sock.addr.sin_port == dst_port) {
            mbuf_pull(msg, sizeof(stcp_udp_header));
            stcp_udp_sockdata d(msg, *src, uh->sport, uh->dport);
            DEBUG("UDP DATA PUSH %u -> %u \n", 
                    rte::bswap16(uh->sport),
                    rte::bswap16(uh->dport));
            sock.rxq.push(d);
            return ;
        }
    }

    DEBUG("\tICMP rx_push: port unreach\n");
    mbuf_push(msg, sizeof(stcp_ip_header));
    core::instance().icmp.send_err(STCP_ICMP_UNREACH, STCP_ICMP_UNREACH_PORT, src, msg);
}



} /* namespace slank */
