

#include "rte.h"
#include "dpdk.h"

#define BURST_SIZE 32




int main(int argc, char** argv)
{
    dpdk::core& dpdk = dpdk::core::instance();
    dpdk.init(argc, argv);
    dpdk::pkt_queue fifo;

    for (;;) {
        for (uint8_t port=0; port<dpdk.num_ports(); port++) {
            struct rte_mbuf* bufs[BURST_SIZE];
            uint16_t num_rx = dpdk.io_rx(port, bufs, BURST_SIZE);

            if (unlikely(num_rx == 0)) continue;
            fifo.enq_array(bufs, num_rx);
            printf("port%u recieved %u packets\n", port, num_rx);
        }
        if (fifo.size() > 10) break;
    }
    fifo.print_info();
}



