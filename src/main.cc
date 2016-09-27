
#include <stcp/stcp.h>

using namespace slank;

static void set_netmask(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4);
static void set_ip_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4);
static void set_hw_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t o5, uint8_t o6);
static void add_arp_record(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4);
static void send_packet_test_eth_mod();
static void add_rtentry();



int main(int argc, char** argv)
{
    core& s = core::instance();  
    s.init(argc, argv);

    set_ip_addr(192, 168, 222, 10);
    set_netmask(255, 255, 255, 0);
    set_hw_addr(0x00, 0x11 , 0x22 , 0x33 , 0x44 , 0x55);
    add_arp_record(192, 168, 222, 1  );
    add_rtentry();

    send_packet_test_eth_mod();
    s.run();
}



/*-------------------*/

static void add_rtentry()
{
    ip_module& ip = core::instance().ip;
    stcp_rtentry rt;

    rt.rt_gateway.inet_addr(192, 168, 222, 100);
    rt.rt_port = 0;
    ip.ioctl(STCP_SIOCADDGW, &rt);

    rt.rt_route.inet_addr(192, 168, 222, 0);
    rt.rt_genmask.inet_addr(255, 255, 255, 0);
    rt.rt_flags = STCP_RTF_MASK | STCP_RTF_LOCAL;
    rt.rt_port = 0;
    ip.ioctl(STCP_SIOCADDRT, &rt);
    
}

static void set_netmask(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4)
{
    dpdk_core& dpdk = core::instance().dpdk;
    struct stcp_ifreq ifr;

    memset(&ifr, 0, sizeof ifr);
    struct stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&ifr.if_addr);
    sin->sin_addr = stcp_inet_addr(o1, o2, o3, o4);
    dpdk.devices[0].ioctl(STCP_SIOCSIFNETMASK, &ifr);
}
static void set_ip_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4)
{
    dpdk_core& dpdk = core::instance().dpdk;
    struct stcp_ifreq ifr;

    memset(&ifr, 0, sizeof ifr);
    struct stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&ifr.if_addr);
    sin->sin_addr = stcp_inet_addr(o1, o2, o3, o4);
    dpdk.devices[0].ioctl(STCP_SIOCSIFADDR, &ifr);
}

static void set_hw_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t o5, uint8_t o6)
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

static void add_arp_record(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4)
{
    struct stcp_arpreq req;
    stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&req.arp_pa);

    req.arp_ifindex = 0;
    req.arp_ha = stcp_inet_hwaddr(0x74, 0x03, 0xbd, 0x13, 0x2c, 0xa6);
    sin->sin_addr = stcp_inet_addr(o1, o2, o3, o4);
    core::instance().arp.ioctl(STCP_SIOCAARPENT, &req);
}

static void send_packet_test_eth_mod()
{
    ether_module& eth = core::instance().ether;

    uint8_t buf[] = {
#if 1
        0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
        0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
        0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
        0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
        0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
        0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee
#else
        
        0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xc0, 0xa8, 0xde, 0x64,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif
    };

    stcp_sockaddr dst;
    stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&dst);
    sin->sin_fam = STCP_AF_INET;
    sin->sin_addr = stcp_inet_addr(192, 168, 222, 100);
    eth.sendto(buf, sizeof(buf), &dst);
}

