
#include <stcp/stcp.h>
#include <stcp/api.h>
#include <stcp/util.h>
#include <unistd.h> // for sleep()
#define UNUSED(x) (void)(x)

using namespace slank;


#if 0
int user_main2(void* arg)
{
    UNUSED(arg);

    stcp_sockaddr_in addr;
    addr.sin_fam  = STCP_AF_INET;
    addr.sin_port = hton16(9999);
    stcp_udp_sock* sock = core::create_udp_socket();
    sock->bind(&addr);

    stcp_sockaddr_in src;
    while (1) {
        mbuf* m = sock->recvfrom(&src);
        sock->sendto(m, &src);
    }
    return 0;
}
#endif



#if 1
// TODO #21
int user_main1(void* arg)
{
    UNUSED(arg);
    stcp_tcp_sock* sock;
    sock = core::create_tcp_socket();

    stcp_sockaddr_in addr;
    addr.sin_fam  = STCP_AF_INET;
    addr.sin_port = hton16(8888);
    sock->bind(&addr, sizeof(addr));
    sock->listen(5);

    std::vector<stcp_tcp_sock*> fds;
    fds.push_back(sock);

    while (true) {
        // printf("[");
        for (size_t i=0; i<fds.size(); i++) {
            // printf("%p, ", fds[i]);
            if (fds[i]->acceptable()) {
                stcp_sockaddr_in caddr;
                stcp_tcp_sock* csock = fds[i]->accept(&caddr);
                fds.push_back(csock);
            } else if (fds[i]->readable()) {
#if 0
                mbuf* msg = fds[i]->read();
                // rte::pktmbuf_dump(stdout, msg, rte::pktmbuf_pkt_len(msg));
                fds[i]->write(msg);
#endif
            } else if (fds[i]->sockdead()) {
                core::destroy_tcp_socket(fds[i]);
                fds.erase(fds.begin() + i);
            } else {
            }
        }
        // printf("]\n");
    }
    return 0;
}
#else
int user_main1(void* arg)
{
    UNUSED(arg);
    stcp_tcp_sock* sock = core::create_tcp_socket();

    stcp_sockaddr_in addr;
    addr.sin_fam  = STCP_AF_INET;
    addr.sin_port = hton16(8888);
    sock->bind(&addr, sizeof(addr));
    sock->listen(5);

    stcp_sockaddr_in caddr;
    stcp_tcp_sock* csock = sock->accept(&caddr);

    while (true) {
        try {
            mbuf* msg = csock->read();
            rte::pktmbuf_dump(stdout, msg, rte::pktmbuf_pkt_len(msg));
            csock->write(msg);
        } catch (std::exception& e) {
            core::destroy_tcp_socket(csock);
            break;
        }

    }
    return 0;
}
#endif

int main(int argc, char** argv)
{
    core::init(argc, argv);

    core::set_hw_addr(0x00, 0x11 , 0x22 , 0x33 , 0x44 , 0x55);
    core::set_ip_addr(192, 168, 222, 10, 24);
    core::set_default_gw(192, 168, 222, 1, 0);

    core::set_app(user_main1, NULL); // TODO #21
    // core::set_app(user_main2, NULL); // TODO #21
    core::run();
}
