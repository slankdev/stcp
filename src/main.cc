
#include <stcp/stcp.h>
#include <stcp/api.h>

using namespace slank;


int main(int argc, char** argv)
{
    core& s = core::instance();  
    s.init(argc, argv);

    set_hw_addr(0x00, 0x11 , 0x22 , 0x33 , 0x44 , 0x55);
    set_ip_addr(192, 168, 222, 10, 24);
    set_default_gw(192, 168, 222, 1, 0);
    // add_arp_record(192, 168, 222, 11,
            // 0x74, 0x03, 0xbd, 0x3d, 0x78, 0x96);
    open_udp_port(9999);

    s.stat_all();
    while (true) {
        s.run(false);
        s.stat_all();
    }
}
