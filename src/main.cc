
#include <stcp/stcp.h>

using namespace slank;

int main(int argc, char** argv)
{
    stcp& s = stcp::instance();  
    s.init(argc, argv);
    


    /* 
     * set ip codes; 
     */
    struct stcp_sockaddr_in sin;
    sin.sin_fam = STCP_AF_INET;
    sin.sin_addr = stcp_inet_addr(192, 168, 222, 254);
    dpdk& dpdk = dpdk::instance();
    dpdk.devices[0].ioctl(stcp_siocsifaddr, &sin);



    s.run();
}

