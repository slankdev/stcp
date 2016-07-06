
#include <stcp/rte.h>
#include <stcp/dpdk.h>
#include <arpa/inet.h>
#include <pgen2.h>
#include <stcp/protocol.h>

#define BURST_SIZE 32
#define ETHER_TYPE(PTR) *(uint16_t*)((uint8_t*)(PTR)+12)
const char* out = "out.pcap";



static void analyze(protocol::proto_module rx)
{
    pgen::pcap_stream pcap(out, pgen::open_mode::pcap_write);

    while (rx.rx_size()>0) {
        struct rte_mbuf* mbuf = rx.rx_out();
        pcap.send(
                rte::mbuf2ptr(mbuf),
                rte::mbuf2len(mbuf));

        if(rte::mbuf2len(mbuf) < 14) {
            printf("len:%zd \n", rte::mbuf2len(mbuf));
            exit(-1);
        }

        if (ETHER_TYPE(rte::mbuf2ptr(mbuf)) == htons(0x0806)) printf("arp\n");
        else if (ETHER_TYPE(rte::mbuf2ptr(mbuf)) == htons(0x0800)) printf("ip\n");
        else if (ETHER_TYPE(rte::mbuf2ptr(mbuf)) == htons(0x86dd)) printf("ip6\n");
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
        }
        if (rx.rx_size() > 10)
            break;
    }

    analyze(rx);
}



