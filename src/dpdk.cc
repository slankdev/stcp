
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <inttypes.h>

#include <stcp/rte.h>
#include <stcp/dpdk.h>



namespace dpdk {


uint16_t core::rx_ring_size = 128;
uint16_t core::tx_ring_size = 512;
uint32_t core::num_mbufs = 8192;
uint32_t core::mbuf_cache_size = 250;
uint16_t core::num_rx_rings = 1;
uint16_t core::num_tx_rings = 1;



core& core::instance()
{
    static core instance;
    return instance;
}

core::core() : mempool(nullptr) {}
core::~core() {}

void core::init(int argc, char** argv)
{
    rte::eth_dev_init(argc, argv);
    
    if (rte::eth_dev_count() < 1) {
        throw rte::exception("num of devices is less than 1");
    }

    mempool = rte::pktmbuf_pool_create(
            "SLANK", 
            num_mbufs * rte::eth_dev_count(), 
            mbuf_cache_size, 
            0, 
            RTE_MBUF_DEFAULT_BUF_SIZE, 
            rte::socket_id()
            );

    for (size_t port=0; port<rte::eth_dev_count(); port++) {
        port_init(port);
    }
}

void core::port_init(uint8_t port)
{

    struct rte_eth_conf port_conf;
    memset(&port_conf, 0, sizeof port_conf);
    port_conf.rxmode.max_rx_pkt_len = ETHER_MAX_LEN;

    rte::eth_dev_configure(port, num_rx_rings, num_tx_rings, &port_conf);

    for (uint16_t ring=0; ring<num_rx_rings; ring++) {
        rte::eth_rx_queue_setup(port, ring, rx_ring_size,
                rte::eth_dev_socket_id(port), NULL, this->mempool); 
    }
    for (uint16_t ring=0; ring<num_tx_rings; ring++) {
        rte::eth_tx_queue_setup(port, ring, tx_ring_size,
                rte::eth_dev_socket_id(port), NULL); 
    }
    rte::eth_dev_start(port);
    rte::eth_promiscuous_enable(port);

    if (rte::eth_dev_socket_id(port) > 0 && 
            rte::eth_dev_socket_id(port) != (int)rte::socket_id()) {
        char str[128];
        sprintf(str, "WARNING: port %4u is on remote NUMA node to "
                "polling thread. \n\tPerformance will "
                "not be optimal. \n ", port);
        throw rte::exception(str);
    }

    net_device dev(port);
    struct ether_addr addr;
    rte::eth_macaddr_get(port, &addr);
    dev.set_hw_addr(&addr);

    devices.push_back(dev);
}


struct rte_mbuf* array2llist_mbuf(struct rte_mbuf** bufs, size_t num_bufs)
{
    if (num_bufs <= 0) return nullptr;

    struct rte_mbuf* link_head = bufs[0];
    struct rte_mbuf* link = link_head;
    for (size_t i=0; i<num_bufs-1; i++) {
        link->next = bufs[i+1];
        link = link->next;
    }
    return link_head;
}




} /* namespace dpdk */




