
#include <stcp/stcp.h>
#include <stcp/api.h>
#include <unistd.h> // for sleep()
#define UNUSED(x) (void)(x)

using namespace slank;




// TODO #21
int user_main1(void* arg)
{
    UNUSED(arg);
    stcp_tcp_sock* sock;
    sock = core::create_tcp_socket();

    stcp_sockaddr_in addr;
    addr.sin_fam  = STCP_AF_INET;
    addr.sin_port = rte::bswap16(8888);
    sock->bind(&addr, sizeof(addr));
    sock->listen(5);

    stcp_sockaddr_in caddr;
    stcp_tcp_sock* csock = sock->accept(&caddr);
    UNUSED(csock);
    // while (true) {
    //     mbuf* msg;
    //     size_t recvlen = csock->recv(msg);
    //     rte::pktmbuf_dump(stdout, msg, 0);
    // }
    return 0;
}



int main(int argc, char** argv)
{
    core::init(argc, argv);

    core::set_hw_addr(0x00, 0x11 , 0x22 , 0x33 , 0x44 , 0x55);
    core::set_ip_addr(192, 168, 222, 10, 24);
    core::set_default_gw(192, 168, 222, 1, 0);

    core::set_app(user_main1, NULL); // TODO #21
    // TcpEchoServer app;
    core::run();
}
