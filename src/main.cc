
#include <stcp/stcp.h>

using namespace slank;


static void start_up()
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

    /* send arp packet */
    uint8_t buf[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xee, 0xee,
        0xee, 0xee, 0xee, 0xee, 0x08, 0x06, 0x00, 0x01,
        0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xc0, 0xa8, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8,
        0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    dpdk.devices[0].write(buf, sizeof(buf));
}


int main(int argc, char** argv)
{
    core& s = core::instance();  
    s.init(argc, argv);
    start_up();
    s.run();
}

