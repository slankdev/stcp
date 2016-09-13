
#include <stcp/stcp.h>

using namespace slank;

int main(int argc, char** argv)
{
    stcp& s = stcp::instance();  
    s.init(argc, argv);
    // s.user_setting();
    
    // set ip codes;
    struct stcp_ip_addr a;
    a.addr_bytes[0] = 192;
    a.addr_bytes[1] = 168;
    a.addr_bytes[2] = 222;
    a.addr_bytes[3] = 254;
    struct stcp_sockaddr_in sin;
    sin.sin_fam = STCP_AF_INET;
    sin.sin_addr = a;
    dpdk& dpdk = dpdk::instance();
    dpdk.devices[0].ioctl(stcp_siocsifaddr, &sin);

    s.run();
}

