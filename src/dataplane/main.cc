

#include "dpdk.h"

#define RX_RING_SIZE 128
#define TX_RING_SIZE 512

#define NUM_MBUFS       8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE      32


static void lcore_main(dpdk& d)
{
    const uint8_t num_ports = d.eth_dev_count();

    for (uint8_t port = 0; port < num_ports; port++) {
        if (rte_eth_dev_socket_id(port) > 0 && rte_eth_dev_socket_id(port) != (int)rte_socket_id())
            printf("WARNING: port %u is on remote NUMA node to "
                    "polling thread. \n\tPerformance will "
                    "not be optimal. \n ", port);
    }
    printf("\nCore %u forwarding packets. [Ctrl+C to quit]\n", rte_lcore_id());


    for (;;) {
        
        for (uint8_t port=0; port<num_ports; port++) {
            struct rte_mbuf* bufs[BURST_SIZE];
            const uint16_t num_rx = rte_eth_rx_burst(port, 0, bufs, BURST_SIZE);

            if (unlikely(num_rx == 0))
                continue;

            for (uint16_t i=0; i<num_rx; i++) {
                rte_pktmbuf_dump(stdout, bufs[i], sizeof(struct rte_mbuf));
                /* rte_hexdump(stdout, "recv packet", */
                /*         rte_pktmbuf_mtod(bufs[i], void*) , */
                /*         rte_pktmbuf_data_len(bufs[i])     ); */
                uint8_t* head = rte_pktmbuf_mtod(bufs[i], uint8_t*);
                memset(head , 0xee, 6);
            }


            const uint16_t num_tx = 0;
            /* const uint16_t num_tx = rte_eth_tx_burst(port, 0, bufs, num_rx); */
            /* printf("Reflect %d packet !! \n", num_rx); */
            if (unlikely(num_tx < num_rx)) {
                uint16_t buf;
                for (buf=num_tx; buf<num_rx; buf++)
                    rte_pktmbuf_free(bufs[buf]);
            }

            printf("\n\n");

        }
    }
}



static int port_init(dpdk& d, uint8_t port)
{
#if 0
    const struct rte_eth_conf port_conf_default = {
            .rxmode = { .max_rx_pkt_len = ETHER_MAX_LEN }
    };
#else
    struct rte_eth_rxmode rxmode;
    memset(&rxmode, 0, sizeof rxmode);
    rxmode.max_rx_pkt_len = ETHER_MAX_LEN;
    struct rte_eth_conf port_conf_default;
    memset(&port_conf_default, 0, sizeof(port_conf_default));
    port_conf_default.rxmode = rxmode;
#endif

    struct rte_eth_conf port_conf = port_conf_default;

    if (port >= d.eth_dev_count()) return -1;

    const uint16_t rx_rings = 1;
    const uint16_t tx_rings = 1;
    d.eth_dev_configure(port, rx_rings, tx_rings, &port_conf);


    for (uint16_t q=0; q < rx_rings; q++)
        d.eth_rx_queue_setup(port, q, RX_RING_SIZE, 
                rte_eth_dev_socket_id(port), NULL);

    for (uint16_t q=0; q < tx_rings; q++)
        d.eth_tx_queue_setup(port, q, TX_RING_SIZE, 
                rte_eth_dev_socket_id(port), NULL);

    d.eth_dev_start(port);

    struct ether_addr addr;
    d.eth_macaddr_get(port, &addr);
    printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
            " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
            (unsigned)port,
            addr.addr_bytes[0], addr.addr_bytes[1],
            addr.addr_bytes[2], addr.addr_bytes[2],
            addr.addr_bytes[3], addr.addr_bytes[4]);

	d.eth_promiscuous_enable(port);

    return 0;
}




int main(int argc, char** argv)
{
    dpdk d;
    d.eth_dev_init(argc, argv);
    
    uint8_t num_ports = d.eth_dev_count();
    printf("%d ports found  \n", num_ports);
    if (num_ports < 1)
        rte_exit(EXIT_FAILURE, "eth_dev_count()");

    d.pktmbuf_pool_create(
            "SLANK", NUM_MBUFS*num_ports, 
            MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, 
            rte_socket_id());

    for (uint8_t portid=0; portid < num_ports; portid++) {
        if (port_init(d, portid) != 0)
            rte_exit(EXIT_FAILURE, "Can't init port%u\n", portid);
    }
    // ここまで完了した。

    lcore_main(d);
}
