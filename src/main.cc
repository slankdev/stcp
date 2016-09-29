
#include <stcp/stcp.h>

using namespace slank;

static void set_netmask(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4);
static void set_ip_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4);
static void set_hw_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t o5, uint8_t o6);
static void set_default_gw(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t port);
static void send_packet_test_ip_mod(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4);

static void add_arp_record(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4,
        uint8_t ho1, uint8_t ho2, uint8_t ho3, uint8_t ho4, uint8_t ho5, uint8_t ho6)
{
    struct stcp_arpreq req;
    stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&req.arp_pa);

    req.arp_ifindex = 0;
    req.arp_ha = stcp_inet_hwaddr(ho1, ho2, ho3, ho4, ho5, ho6);
    sin->sin_addr = stcp_inet_addr(o1, o2, o3, o4);
    core::instance().arp.ioctl(STCP_SIOCAARPENT, &req);
}



int main(int argc, char** argv)
{
    core& s = core::instance();  
    s.init(argc, argv);

    set_ip_addr(192, 168, 222, 10);
    set_netmask(255, 255, 255, 0);
    set_hw_addr(0x00, 0x11 , 0x22 , 0x33 , 0x44 , 0x55);
    add_arp_record(192, 168, 222, 100, 
            0xff, 0xff , 0xff , 0xff , 0xff , 0xff);
    set_default_gw(192, 168, 222, 1, 0);

    
    send_packet_test_ip_mod(192, 168, 222, 100);
    s.run();
}

static void send_packet_test_ip_mod(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4)
{
    ip_module& ip = core::instance().ip;

    uint8_t buf[] = {
        // #<{(| ip hdr |)}>#
        // 0x45, 0x00, 0x00, 0x54, 0x7e, 0x4d, 0x40, 0x00, 
        // 0x40, 0x01, 0x7e, 0x9a, 0xc0, 0xa8, 0xde, 0x0b, 
        // 0xc0, 0xa8, 0xde, 0x64, 

        /* icmp hdr */
        0x08, 0x00, 0x39, 0x86, 0x60, 0xd7, 0x00, 0x01, 
        0x59, 0xf4, 0xe9, 0x57, 0x00, 0x00, 0x00, 0x00, 
        0x54, 0x82, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 
        
        /* icmp data */
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
        0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 
        0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
        0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 
        0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
        0x36, 0x37,
    };

    stcp_sockaddr dst;
    dst.sa_fam = STCP_AF_INET;
    dst.inet_addr(o1, o2, o3, o4);
    ip.sendto(buf, sizeof(buf), &dst);
}



/*-------------------*/

static void set_default_gw(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t port)
{
    ip_module& ip = core::instance().ip;

    stcp_rtentry rt;
    rt.rt_gateway.inet_addr(o1, o2, o3, o4);
    rt.rt_port = port;
    ip.ioctl(STCP_SIOCADDGW, &rt);
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
