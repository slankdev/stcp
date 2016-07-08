

#include <stcp/rte.h>
#include <stcp/dpdk.h>
#include <stcp/protocol.h>
#include <arpa/inet.h>

#define BURST_SIZE 32
#define ETHER_TYPE(PTR) *(uint16_t*)((uint8_t*)(PTR)+12)



static void analyze(protocol::proto_module& rx)
{
    while (rx.rx_size()>0) {
        struct rte_mbuf* mbuf = rx.rx_out();

        if(rte::mbuf2len(mbuf) < 14) {
            printf("length: %zd\n", rte::mbuf2len(mbuf));
            continue ;
        }

        static int c=0; c++; printf("%04d: ", c);

        uint16_t type = htons( ETHER_TYPE(rte::mbuf2ptr(mbuf)) );
        if (type == 0x0806)      printf("arp\n");
        else if (type == 0x0800) printf("ip\n");
        else if (type == 0x86dd) printf("ip6\n");
        else printf("other \n");

        rte::pktmbuf_free(mbuf);
    }
}



int main(int argc, char** argv)
{
    dpdk::core& dpdk = dpdk::core::instance();
    dpdk.init(argc, argv);
    protocol::proto_module rx;

    for (;;) {
        for (uint8_t port=0; port<dpdk.num_ports(); port++) {
            struct rte_mbuf* bufs[BURST_SIZE];
            uint16_t num_rx = dpdk.io_rx(port, bufs, BURST_SIZE);

            if (unlikely(num_rx == 0)) continue;
            rx.rx_in(dpdk::pkt_queue::array2llist(bufs, num_rx));
            analyze(rx);
        }
    }
}



