
#include <stcp/stcp.h>

using namespace slank;


static void start_up()
{
    dpdk& dpdk = dpdk::instance();
    core& s = core::instance();  
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


    /* add arp-record */
    struct stcp_arpreq req;
    req.arp_ifindex = 0;
    req.arp_ha.sa_data[0] = 0xee;
    req.arp_ha.sa_data[1] = 0xee;
    req.arp_ha.sa_data[2] = 0xee;
    req.arp_ha.sa_data[3] = 0xee;
    req.arp_ha.sa_data[4] = 0xee;
    req.arp_ha.sa_data[5] = 0xee;
    sin = reinterpret_cast<stcp_sockaddr_in*>(&req.arp_pa);
    sin->sin_addr = stcp_inet_addr(192, 168, 222, 111);
    s.arp.ioctl(STCP_SIOCAARPENT, &req);

    sin->sin_addr = stcp_inet_addr(192, 168, 222, 112);
    s.arp.ioctl(STCP_SIOCAARPENT, &req);
    sin->sin_addr = stcp_inet_addr(192, 168, 222, 113);
    s.arp.ioctl(STCP_SIOCAARPENT, &req);

    /* del arp-record */
    // req.arp_ifindex = 0;
    // sin->sin_addr = stcp_inet_addr(192, 168, 222, 111);
    // s.arp.ioctl(STCP_SIOCDARPENT, &req);

    std::vector<stcp_arpreq>* tbl;
    s.arp.ioctl(STCP_SIOCGARPENT, &tbl);
    for (size_t i=0; i<tbl->size(); i++) {
        printf("%zd: %s %d\n", i, p_sockaddr_to_str(&(*tbl)[i].arp_pa), 
                (*tbl)[i].arp_ifindex);
    }
    exit(-1);
}


int main(int argc, char** argv)
{
    core& s = core::instance();  
    s.init(argc, argv);

    start_up();

    s.run();
}

