


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

#include "dpdk.h"




dpdk::dpdk() : mbuf_pool(nullptr)
{
    printf("Init DPDK \n");
}
dpdk::~dpdk()
{
    printf("destroy DPDK \n");
}




void dpdk::eth_dev_init(int argc, char** argv)
{
    int ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "rte_eal_init() failed\n");
}

void dpdk::eth_dev_configure(uint8_t port_id, uint16_t nb_rx_queue,
        uint16_t nb_tx_queue, const struct rte_eth_conf* eth_conf)
{
    int ret = rte_eth_dev_configure(port_id, nb_rx_queue, nb_tx_queue, eth_conf);
    if (ret != 0) {
        rte_exit(EXIT_FAILURE, "eth_dev_configure()");
    }
}

void dpdk::eth_rx_queue_setup(uint8_t port_id, uint16_t rx_queue_id, 
        uint16_t nb_rx_desc, uint32_t socket_id, 
        const struct rte_eth_rxconf* rx_conf)
{
    int ret = rte_eth_rx_queue_setup(port_id, rx_queue_id, 
            nb_rx_desc, socket_id, rx_conf, mbuf_pool);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "rx_queue_setup()");
    }
}

void dpdk::eth_tx_queue_setup(uint8_t port_id, uint16_t tx_queue_id, 
        uint16_t nb_tx_desc, uint32_t socket_id, 
        const struct rte_eth_txconf* tx_conf)
{
    int ret = rte_eth_tx_queue_setup(port_id, tx_queue_id, 
            nb_tx_desc, socket_id, tx_conf);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "tx_queue_setup()");
    }
}

void dpdk::eth_dev_start(uint8_t port_id)
{
    int ret = rte_eth_dev_start(port_id);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "rte_eth_dev_start() returned %d", ret);
}

void dpdk::eth_promiscuous_enable(uint8_t port_id)
{
    rte_eth_promiscuous_enable(port_id);
}

void dpdk::pktmbuf_pool_create(const char* name, uint32_t n, 
        uint32_t cache_size, uint16_t priv_size, 
        uint16_t data_room_size, int socket_id)
{
    mbuf_pool = rte_pktmbuf_pool_create(
            name, n, cache_size, priv_size, data_room_size, socket_id);
    if (mbuf_pool == NULL)
        rte_exit(EXIT_FAILURE, "pktmbuf_pool_create()");

}

size_t dpdk::eth_dev_count()
{
    return rte_eth_dev_count();
}

size_t dpdk::lcore_count()
{
    return rte_lcore_count();
}

void dpdk::eth_macaddr_get(uint8_t port_id, struct ether_addr* mac_addr)
{
    rte_eth_macaddr_get(port_id, mac_addr);
}

