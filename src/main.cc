
#include <stcp/stcp.h>
#include <stcp/api.h>

using namespace slank;

# if 0
class UdpEchoServer : public stcp_app {
    stcp_udp_sock* s;
    size_t recv_count;

    friend class cf;
    class cf : public stcp_cyclic_func {
    public:
        UdpEchoServer* parent;
        cf(uint64_t ms, UdpEchoServer* p)
            : stcp_cyclic_func(ms), parent(p) {}
        void exec() override
        {
            printf("recv_count: %zd \n", parent->recv_count);
        }
    } f;

public:
    UdpEchoServer() : stcp_app(), recv_count(0), f(1000, this)
    {
        stcp_sockaddr_in addr;
        addr.sin_fam  = STCP_AF_INET;
        addr.sin_port = rte::bswap16(9999);
        stcp_udp_sock& sock = stcp_udp_sock::socket();
        s = &sock;
        sock.bind(&addr);

        core::add_cyclic(&f);
    }
    void proc() override
    {
        stcp_sockaddr_in src;
        mbuf* m = s->recvfrom(&src);
        if (m) {
            recv_count++;
            s->sendto(m, &src);
        }
    }
};
#endif


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

    TcpEchoServer app;
    core::run();
}
