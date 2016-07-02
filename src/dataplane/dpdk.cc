
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <inttypes.h>

#include <rte_config.h>
#include <rte_version.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_hexdump.h>

#include "rte.h"
#include "dpdk.h"


#define RX_RING_SIZE 128
#define TX_RING_SIZE 512

#define NUM_MBUFS       8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE      32



namespace dpdk {



core::core() : mempool(nullptr) {}
void core::init(int argc, char** argv)
{
    rte::eth_dev_init(argc, argv);
    
    if (rte::eth_dev_count() < 1) {
        throw rte::exception("num of devices is less than 1");
    }

    mempool = rte::pktmbuf_pool_create(
            "SLANK", 
            NUM_MBUFS * rte::eth_dev_count(), 
            MBUF_CACHE_SIZE, 
            0, 
            RTE_MBUF_DEFAULT_BUF_SIZE, 
            rte::socket_id()
            );

    ports_init();
}




void core::ports_init()
{
    dpdk::conf conf;

    struct rte_eth_conf port_conf = conf.port_conf;
    const uint16_t nb_ports = rte::eth_dev_count();

    for (uint16_t port=0; port<nb_ports; port++) {
        const uint16_t rx_rings = 1;
        const uint16_t tx_rings = 1;
        rte::eth_dev_configure(port, rx_rings, tx_rings, &port_conf);

        for (uint16_t ring=0; ring<rx_rings; ring++) {
            rte::eth_rx_queue_setup(port, ring, RX_RING_SIZE,
                rte::eth_dev_socket_id(port), NULL, this->mempool); 
        }
        for (uint16_t ring=0; ring<tx_rings; ring++) {
            rte::eth_tx_queue_setup(port, ring, TX_RING_SIZE,
                rte::eth_dev_socket_id(port), NULL); 
        }
        rte::eth_dev_start(port);
        rte::eth_promiscuous_enable(port);
    }
}


    

} /* namespace dpdk */


