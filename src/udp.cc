

#include <stcp/udp.h>
#include <stcp/stcp.h>


namespace slank {


void udp_module::rx_push(mbuf* msg, const stcp_sockaddr* src)
{

    stcp_udp_header* uh
        = rte::pktmbuf_mtod<stcp_udp_header*>(msg);

    uint16_t dst_port = rte::bswap16(uh->dport);
    DEBUG("ICMP rx_push from %s to port%u \n", src->c_str(), dst_port);

    for (stcp_udp_sock& sock : socks) {
        if (sock.port == dst_port) {
            stcp_udp_sockdata d(msg, *src);
            sock.rxq.push(d);
            return ;
        }
    }

    DEBUG("\tICMP rx_push: port unreach\n");
    mbuf_push(msg, sizeof(stcp_ip_header));
    core::instance().icmp.send_err(STCP_ICMP_UNREACH, STCP_ICMP_UNREACH_PORT, src, msg);
}



void udp_module::ioctl(uint64_t request, void* args)
{
    switch (request) {
        case STCP_SIOCOPENUDPPORT:
        {
            uint16_t* port = reinterpret_cast<uint16_t*>(args);
            ioctl_siocopenudpport(port);
            break;
        }
        case STCP_SIOCCLOSEUDPPORT:
        {
            uint16_t* port = reinterpret_cast<uint16_t*>(args);
            ioctl_sioccloseudpport(port);
            break;
        }
        default:
        {
            throw exception("invalid arguments");
            break;
        }
    }
}


void udp_module::ioctl_siocopenudpport(const uint16_t* port)
{
    stcp_udp_sock sock(*port);
    socks.push_back(sock);
}


void udp_module::ioctl_sioccloseudpport(const uint16_t* port)
{
    for (size_t i=0; i<socks.size(); i++) {
        if (socks[i].port == *port) {
            socks.erase(socks.begin() + i);
            return ;
        }
    }
    throw slankdev::exception("no such port opening");
}


} /* namespace slank */
