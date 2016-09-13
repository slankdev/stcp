
#include <stcp/stcp.h>

using namespace slank;

int main(int argc, char** argv)
{
    core& s = core::instance();  
    s.init(argc, argv);
    

    /* 
     * set ip codes; 
     */
    struct stcp_ifreq ifr;
    struct stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&ifr.if_addr);
    sin->sin_addr = stcp_inet_addr(192, 168, 222, 254);
    dpdk& dpdk = dpdk::instance();
    dpdk.devices[0].ioctl(stcp_siocsifaddr, &ifr);



    s.run();
}

