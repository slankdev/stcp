

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>

#include <string>
#include <iostream>
#include <sstream>
#include <exception>
#include <vector>

#include <stcp/config.h>
#include <stcp/dpdk.h>
#include <stcp/net_device.h>
#include <stcp/rte.h>


uint16_t net_device::rx_ring_size = 128;
uint16_t net_device::tx_ring_size = 512;
uint16_t net_device::num_rx_rings = 1;
uint16_t net_device::num_tx_rings = 1;



void net_device::init()
{
    log& log = log::instance();
    log.push(name.c_str());

    struct rte_eth_conf port_conf;
    memset(&port_conf, 0, sizeof port_conf);
    port_conf.rxmode.max_rx_pkt_len = ETHER_MAX_LEN;
    rte::eth_dev_configure(port_id, num_rx_rings, num_tx_rings, &port_conf);

    dpdk& d = dpdk::instance();
    for (uint16_t ring=0; ring<num_rx_rings; ring++) {
        rte::eth_rx_queue_setup(port_id, ring, rx_ring_size,
                rte::eth_dev_socket_id(port_id), NULL, d.get_mempool()); 
    }
    for (uint16_t ring=0; ring<num_tx_rings; ring++) {
        rte::eth_tx_queue_setup(port_id, ring, tx_ring_size,
                rte::eth_dev_socket_id(port_id), NULL); 
    }
    rte::eth_dev_start(port_id);
    rte::eth_promiscuous_enable(port_id);

    if (rte::eth_dev_socket_id(port_id) > 0 && 
            rte::eth_dev_socket_id(port_id) != (int)rte::socket_id()) {
        char str[128];
        sprintf(str, "WARNING: port %4u is on remote NUMA node to "
                "polling thread. \n\tPerformance will "
                "not be optimal. \n ", port_id);
        throw rte::exception(str);
    }

    if_addr ifaddr(AF_LINK);
    struct ether_addr addr;
    rte::eth_macaddr_get(port_id, &addr);
    ifaddr.init(&addr, sizeof(addr));
    addrs.push_back(ifaddr);

    log.pop();
}


