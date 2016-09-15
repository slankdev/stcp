
#include <stcp/stcp.h>

using namespace slank;


static void start_up()
{
    // core& s = core::instance();  
    dpdk& dpdk = dpdk::instance();
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


int main(int argc, char** argv)
{
    core& s = core::instance();  
    s.init(argc, argv);

    start_up();

    s.run();
}

