
#include <stcp/stcp.h>
#include <stcp/api.h>


namespace slank {



void add_arp_record(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4,
        uint8_t ho1, uint8_t ho2, uint8_t ho3, uint8_t ho4, uint8_t ho5, uint8_t ho6)
{
    struct stcp_arpreq req;
    stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&req.arp_pa);

    req.arp_ifindex = 0;
    req.arp_ha = stcp_inet_hwaddr(ho1, ho2, ho3, ho4, ho5, ho6);
    sin->sin_addr = stcp_inet_addr(o1, o2, o3, o4);
    core::instance().arp.ioctl(STCP_SIOCAARPENT, &req);
}


void set_default_gw(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t port)
{
    ip_module& ip = core::instance().ip;

    stcp_rtentry rt;
    rt.rt_gateway.inet_addr(o1, o2, o3, o4);
    rt.rt_port = port;
    ip.ioctl(STCP_SIOCADDGW, &rt);
}

void set_netmask(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4)
{
    dpdk_core& dpdk = core::instance().dpdk;
    struct stcp_ifreq ifr;

    memset(&ifr, 0, sizeof ifr);
    struct stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&ifr.if_addr);
    sin->sin_addr = stcp_inet_addr(o1, o2, o3, o4);
    dpdk.devices[0].ioctl(STCP_SIOCSIFNETMASK, &ifr);
}
void set_ip_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4)
{
    dpdk_core& dpdk = core::instance().dpdk;
    struct stcp_ifreq ifr;

    memset(&ifr, 0, sizeof ifr);
    struct stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&ifr.if_addr);
    sin->sin_addr = stcp_inet_addr(o1, o2, o3, o4);
    dpdk.devices[0].ioctl(STCP_SIOCSIFADDR, &ifr);
}

void set_hw_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t o5, uint8_t o6)
{
    dpdk_core& dpdk = core::instance().dpdk;
    struct stcp_ifreq ifr;

    memset(&ifr, 0, sizeof ifr);
    ifr.if_hwaddr.sa_data[0] = o1;
    ifr.if_hwaddr.sa_data[1] = o2;
    ifr.if_hwaddr.sa_data[2] = o3;
    ifr.if_hwaddr.sa_data[3] = o4;
    ifr.if_hwaddr.sa_data[4] = o5;
    ifr.if_hwaddr.sa_data[5] = o6;
    dpdk.devices[0].ioctl(STCP_SIOCSIFHWADDR, &ifr);
}



} /* namespace slank */
