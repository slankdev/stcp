

#include "rte.h"
#include "dpdk.h"
#define BURST_SIZE      32



static void lcore_main()
{
    const uint8_t num_ports = rte::eth_dev_count();

    for (uint8_t port = 0; port < num_ports; port++) {
        if (rte::eth_dev_socket_id(port) > 0 && 
                rte::eth_dev_socket_id(port) != (int)rte::socket_id()) {
            printf("WARNING: port %u is on remote NUMA node to "
                    "polling thread. \n\tPerformance will "
                    "not be optimal. \n ", port);
        }
    }

    for (;;) {
        for (uint8_t port=0; port<num_ports; port++) {
            struct rte_mbuf* bufs[BURST_SIZE];
            const uint16_t num_rx = rte::eth_rx_burst(port, 0, bufs, BURST_SIZE);

            if (unlikely(num_rx == 0)) continue;

            for (uint16_t i=0; i<num_rx; i++) {
                rte::pktmbuf_dump(stdout, bufs[i], sizeof(struct rte_mbuf));
                uint8_t* head = rte_pktmbuf_mtod(bufs[i], uint8_t*);
                memset(head , 0xee, 6);
            }

            const uint16_t num_tx = rte::eth_tx_burst(port, 0, bufs, num_rx);
            printf("Reflect %d packet !! \n", num_rx);

            if (unlikely(num_tx < num_rx)) {
                uint16_t buf;
                for (buf=num_tx; buf<num_rx; buf++)
                    rte::pktmbuf_free(bufs[buf]);
            }
            printf("\n\n");
        }
    }
}




int main(int argc, char** argv)
{
    dpdk::core dpdk;
    dpdk.init(argc, argv);
    lcore_main();
}
