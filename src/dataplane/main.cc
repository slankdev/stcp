

#include "rte.h"
#include "dpdk.h"

#define BURST_SIZE 32

static void print_mbuf_links(const struct rte_mbuf* mbuf, size_t depth=0)
{
    for (size_t i=0; i<depth; i++) printf("  ");
    printf("[node] %p (next=%p) \n", mbuf, mbuf->next);

    if (mbuf->next) {
        print_mbuf_links(mbuf->next, depth+1);
    }

}


static struct rte_mbuf* make_mbuf_linklist(struct rte_mbuf** bufs, uint16_t size)
{
    struct rte_mbuf* mbuf_ll_head = bufs[0];

    struct rte_mbuf* mbuf_ll = mbuf_ll_head;
    for (uint16_t i=0; i<size-1; i++) {
        mbuf_ll->next = bufs[i+1];
        mbuf_ll = mbuf_ll->next;
    }
    return mbuf_ll_head;
}



static void analyze_packets(uint8_t port, struct rte_mbuf** bufs, uint16_t num_rx)
{
    if (unlikely(num_rx == 0)) return;

    printf("recv %d packets from port%u\n", num_rx, port);
    struct rte_mbuf* mbuf_ll = make_mbuf_linklist(bufs, num_rx);

    print_mbuf_links(bufs[0]);
    do {
        struct rte_mbuf* next = mbuf_ll->next;
        rte::pktmbuf_free(mbuf_ll);
        mbuf_ll = next;
    } while(mbuf_ll);

    printf("\n\n");
}



int main(int argc, char** argv)
{
    dpdk::core dpdk;
    dpdk.init(argc, argv);

    for (;;) {
        for (uint8_t port=0; port<dpdk.num_ports(); port++) {
            struct rte_mbuf* bufs[BURST_SIZE];
            uint16_t num_rx = dpdk.io_rx(port, bufs, BURST_SIZE);
            analyze_packets(port, bufs, num_rx);
        }
    }
}
