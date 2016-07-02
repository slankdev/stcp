
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





namespace rte {



void eth_dev_init(int argc, char** argv)
{
    int ret = rte_eal_init(argc, argv);
    if (ret < 0) {
        printf("throw \n");
        throw rte::exception("rte_eal_init");
    }
}

void eth_dev_configure(uint8_t port_id, uint16_t nb_rx_queue,
        uint16_t nb_tx_queue, const struct rte_eth_conf* eth_conf)
{
    int ret = rte_eth_dev_configure(port_id, nb_rx_queue, nb_tx_queue, eth_conf);
    if (ret != 0) {
        throw rte::exception("eth_eth_configure");
    }
}

void eth_rx_queue_setup(uint8_t port_id, uint16_t rx_queue_id, 
        uint16_t nb_rx_desc, uint32_t socket_id, 
        const struct rte_eth_rxconf* rx_conf, struct rte_mempool* mbuf_pool)
{
    int ret = rte_eth_rx_queue_setup(port_id, rx_queue_id, 
            nb_rx_desc, socket_id, rx_conf, mbuf_pool);
    if (ret < 0) {
        throw rte::exception("rx_queue_setup");
    }
}

void eth_tx_queue_setup(uint8_t port_id, uint16_t tx_queue_id, 
        uint16_t nb_tx_desc, uint32_t socket_id, 
        const struct rte_eth_txconf* tx_conf)
{
    int ret = rte_eth_tx_queue_setup(port_id, tx_queue_id, 
            nb_tx_desc, socket_id, tx_conf);
    if (ret < 0) {
        throw rte::exception("tx_queue_setup");
    }
}

void eth_dev_start(uint8_t port_id)
{
    int ret = rte_eth_dev_start(port_id);
    if (ret < 0) {
        throw rte::exception("rte_eth_dev_start");
    }
}

void eth_promiscuous_enable(uint8_t port_id)
{
    rte_eth_promiscuous_enable(port_id);
}

void eth_promiscuous_disable(uint8_t port_id)
{
    rte_eth_promiscuous_disable(port_id);
}

int eth_promiscuous_get(uint8_t port_id)
{
    return rte_eth_promiscuous_get(port_id);
}


struct rte_mempool* pktmbuf_pool_create(const char* name, uint32_t n, 
        uint32_t cache_size, uint16_t priv_size, 
        uint16_t data_room_size, int socket_id)
{
    rte_mempool* mbuf_pool = rte_pktmbuf_pool_create(
            name, n, cache_size, priv_size, data_room_size, socket_id);
    if (mbuf_pool == NULL)
        rte_exit(EXIT_FAILURE, "pktmbuf_pool_create()");
    return mbuf_pool;
}

size_t eth_dev_count()
{
    return rte_eth_dev_count();
}

size_t lcore_count()
{
    return rte_lcore_count();
}

void eth_macaddr_get(uint8_t port_id, struct ether_addr* mac_addr)
{
    rte_eth_macaddr_get(port_id, mac_addr);
}

uint16_t eth_rx_burst(uint8_t port_id, uint16_t queue_id, 
        struct rte_mbuf** rx_pkts, const uint16_t nb_pkts)
{
    uint16_t num_rx = rte_eth_rx_burst(port_id, queue_id, rx_pkts, nb_pkts);
    return num_rx;
}

uint16_t eth_tx_burst(uint8_t port_id, uint16_t queue_id, 
        struct rte_mbuf** tx_pkts, uint16_t nb_pkts)
{
    uint16_t num_tx = rte_eth_tx_burst(port_id, queue_id, tx_pkts, nb_pkts);
    return num_tx;
}

void pktmbuf_free(struct rte_mbuf* m)
{
    rte_pktmbuf_free(m);
}

uint32_t socket_id()
{
    return rte_socket_id();
}

void pktmbuf_dump(FILE* f,const struct rte_mbuf* m, unsigned dump_len)
{
    rte_pktmbuf_dump(f, m, dump_len);
}

int eth_dev_socket_id(uint8_t port_id)
{
    return rte_eth_dev_socket_id(port_id);
}



} /* namespace rte */


