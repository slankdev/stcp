
#include <stcp/stcp.h>
#include <stcp/api.h>

using namespace slank;



void test_udp_app(stcp_udp_sock& sock)
{
    stcp_sockaddr_in src;
    mbuf* m = sock.recvfrom(&src);
    if (m) {
        sock.sendto(m, &src);
    }
}


int main(int argc, char** argv)
{
    core& s = core::instance();  
    s.init(argc, argv);

    set_hw_addr(0x00, 0x11 , 0x22 , 0x33 , 0x44 , 0x55);
    set_ip_addr(192, 168, 222, 10, 24);
    set_default_gw(192, 168, 222, 1, 0);
    // add_arp_record(192, 168, 222, 11,
    //         0x74, 0x03, 0xbd, 0x3d, 0x78, 0x96);

    stcp_sockaddr_in addr;
    addr.sin_fam  = STCP_AF_INET;
    addr.sin_port = rte::bswap16(9999);
    stcp_udp_sock& sock = core::instance().udp.socket();
    sock.bind(&addr);


    s.stat_all();
    while (true) {
        test_udp_app(sock);
        s.run(false);
        s.stat_all();
    }
}
