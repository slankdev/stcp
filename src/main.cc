
#include <stcp/stcp.h>
#include <stcp/api.h>

using namespace slank;



class TcpEchoServer : public stcp_app {
    stcp_tcp_sock* sock;
public:
    TcpEchoServer() : stcp_app()
    {
        sock = core::create_tcp_socket();

        stcp_sockaddr_in addr;
        addr.sin_fam  = STCP_AF_INET;
        addr.sin_port = rte::bswap16(9999);
        sock->bind(&addr, sizeof(addr));
        sock->listen(5);

    }
    void proc() override
    {
    }
};


// static int user_main(stcp_usrapp_arg* arg) { // TODO #21
//     #<{(|
//      * int argc    = arg->argc;
//      * char** argv = arg->argv;
//      |)}>#
//     return 0;
// }



int main(int argc, char** argv)
{
    core::init(argc, argv);

    core::set_hw_addr(0x00, 0x11 , 0x22 , 0x33 , 0x44 , 0x55);
    core::set_ip_addr(192, 168, 222, 10, 24);
    core::set_default_gw(192, 168, 222, 1, 0);

#if 1 // for test
    core::add_arp_record(192, 168, 222, 11,
            0x74, 0x03, 0xbd, 0x3d, 0x78, 0x96);
#endif

    // core::set_app(user_main, NULL); // TODO #21
    TcpEchoServer app;
    core::run();
}
