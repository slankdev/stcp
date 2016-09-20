
#include <stcp/stcp.h>

using namespace slank;


static void set_addr()
{
    dpdk_core& dpdk = core::instance().dpdk;
    struct stcp_sockaddr_in* sin;

    /* set ip addr */
    struct stcp_ifreq ifr;
    memset(&ifr, 0, sizeof ifr);
    sin = reinterpret_cast<stcp_sockaddr_in*>(&ifr.if_addr);
    sin->sin_addr = stcp_inet_addr(192, 168, 222, 10);
    dpdk.devices[0].ioctl(STCP_SIOCSIFADDR, &ifr);

    /* set hw addr */
    memset(&ifr, 0, sizeof ifr);
    ifr.if_hwaddr.sa_data[0] = 0x00;
    ifr.if_hwaddr.sa_data[1] = 0x11;
    ifr.if_hwaddr.sa_data[2] = 0x22;
    ifr.if_hwaddr.sa_data[3] = 0x33;
    ifr.if_hwaddr.sa_data[4] = 0x44;
    ifr.if_hwaddr.sa_data[5] = 0x55;
    dpdk.devices[0].ioctl(STCP_SIOCSIFHWADDR, &ifr);
}



static void add_arp_record()
{
    struct stcp_arpreq req;
    stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&req.arp_pa);

    req.arp_ifindex = 0;
    req.arp_ha = stcp_inet_hwaddr(0xee, 0xee, 0xee, 0xee, 0xee, 0xee);
    sin->sin_addr = stcp_inet_addr(192, 168, 222, 111);
    core::instance().arp.ioctl(STCP_SIOCAARPENT, &req);

    req.arp_ifindex = 0;
    req.arp_ha = stcp_inet_hwaddr(0x74, 0x03, 0xbd, 0x3d, 0x78, 0x96);
    sin->sin_addr = stcp_inet_addr(192, 168, 222, 100);
    core::instance().arp.ioctl(STCP_SIOCAARPENT, &req);

}


static void arp_resolv_test()
{
    uint8_t ha[6];
    stcp_sockaddr pa;
    stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&pa);
    sin->sin_addr = stcp_inet_addr(192, 168, 222, 100);

    arp_module&  a = core::instance().arp;
    a.arp_resolv(0, &pa, ha);

    for (int i=0; i<6; i++)
        printf("%02x:", ha[i]);
    printf("\n");
    exit(-1);
}


int main(int argc, char** argv)
{
    try {
        core& s = core::instance();  
        s.init(argc, argv);

        /* start up routines */
        set_addr();
        add_arp_record();

        arp_resolv_test();

        s.run();
    } catch (std::exception& e) {
        printf("%s \n", e.what());
    }
}

