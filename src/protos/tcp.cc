

#include <assert.h>
#include <stcp/protos/tcp.h>
#include <stcp/mempool.h>
#include <stcp/config.h>
#include <stcp/arch/dpdk/device.h>
#include <stcp/protos/tcp_util.h>
#include <stcp/tuning.h>
#define UNUSED(x) (void)(x)

namespace slank {

size_t tcp_module::mss = 1460;



void tcp_module::init()
{
    mp = pool_create(
            "TCP Mem Pool",
            ST_TCPMODULE_MEMPOOL_NSEG * eth_dev_count(),
            ST_TCPMODULE_MP_CACHESIZ,
            0,
            MBUF_DEFAULT_BUF_SIZE,
            cpu_socket_id());
}


/*
 * msg's head must points ip-header
 */
void tcp_module::tx_push(mbuf* msg, const stcp_sockaddr_in* dst)
{
    mbuf_pull(msg, sizeof(stcp_ip_header));
    core::ip.tx_push(msg, dst, STCP_IPPROTO_TCP);
}


void tcp_module::proc()
{
    for (size_t i=0; i<socks.size(); i++) {
        socks[i].proc();
    }
}




void tcp_module::print_stat() const
{
    size_t rootx = screen.POS_TCP.x;
    size_t rooty = screen.POS_TCP.y;
    screen.move(rooty, rootx);

    screen.printwln("TCP module");
    screen.printwln(" Pool: %u/%u", pool_use_count(mp), pool_size(mp));

    if (!socks.empty()) {
        screen.printwln(" NetStat %zd ports", socks.size());
    }

    for (size_t i=0; i<socks.size(); i++) {
        socks[i].print_stat(rootx, 8*i + rooty+3);
    }
}



void tcp_module::rx_push(mbuf* msg, stcp_sockaddr_in* src)
{
    stcp_tcp_header* th = mbuf_mtod<stcp_tcp_header*>(msg);

    bool find_socket = false;
    uint16_t dst_port = th->dport;
    for (stcp_tcp_sock& sock : socks) {
        if (sock.port == dst_port) {
            if (sock.rxq.size() > 1000) { // TODO super hardcode
                for (int i=0; i<100; i++) // TODO super hardcode
                    mbuf_free(sock.rxq.pop());
            }
            mbuf* m = mbuf_clone(msg, core::tcp.mp);
            mbuf_push(m, sizeof(stcp_ip_header));
            sock.rx_push(m, src);
            find_socket = true;
        }
    }

    if (!find_socket) {
        mbuf_push(msg, sizeof(stcp_ip_header));
        tcpip* tih = mtod_tih(msg);

        /*
         * Delete TCP Option field
         */
        mbuf_trim(msg, opt_len(tih));

        /*
         * Set TCP/IP hdr
         */
        tih->ip.src           = tih->ip.dst;
        tih->ip.dst           = src->sin_addr;
        tih->ip.next_proto_id = STCP_IPPROTO_TCP;
        tih->ip.total_length  = hton16(mbuf_pkt_len(msg));
        swap_port(tih);
        tih->tcp.ack      = tih->tcp.seq + hton32(1);
        tih->tcp.seq      = 0;
        tih->tcp.data_off = sizeof(stcp_tcp_header)/4 << 4;
        tih->tcp.flags    = TCPF_RST|TCPF_ACK;
        tih->tcp.rx_win   = 0;
        tih->tcp.cksum    = 0x0000;
        tih->tcp.urp      = 0x0000;

        tih->tcp.cksum = cksum_tih(tih);
        core::tcp.tx_push(mbuf_clone(msg, core::tcp.mp), src);
    }
    mbuf_free(msg);
    return;
}




} /* namespace slank */
