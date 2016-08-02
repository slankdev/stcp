

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
#include <stcp/ifnet.h>
#include <stcp/rte.h>









void ifnet::init()
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

    if (promiscuous_mode)
        rte::eth_promiscuous_enable(port_id);

    if (rte::eth_dev_socket_id(port_id) > 0 && 
            rte::eth_dev_socket_id(port_id) != (int)rte::socket_id()) {
        char str[128];
        sprintf(str, "WARNING: port %4u is on remote NUMA node to "
                "polling thread. \n\tPerformance will "
                "not be optimal. \n ", port_id);
        throw rte::exception(str);
    }

    ifaddr ifa(AF_LINK);
    struct ether_addr addr;
    rte::eth_macaddr_get(port_id, &addr);
    ifa.init(&addr, sizeof(addr));
    addrs.push_back(ifa);

    log.pop();
}


uint16_t ifnet::io_rx()
{
    struct rte_mbuf* bufs[BURST_SIZE];
    uint16_t num_rx = rte::eth_rx_burst(port_id, 0, bufs, BURST_SIZE);
    if (unlikely(num_rx == 0)) return 0;

    rx.push(array2llist_mbuf(bufs, num_rx));
    rx_packets += num_rx;
    return num_rx;
}
uint16_t ifnet::io_tx(size_t num_request_to_send)
{

    if (num_request_to_send > tx.size()) {
        num_request_to_send = tx.size();
    }

    struct rte_mbuf* bufs[BURST_SIZE];

    uint16_t num_tx_sum = 0;
    size_t i=0;
    for (size_t num_sent=0; num_sent<num_request_to_send; num_sent+=i) {
        for (i=0; i+num_sent<num_request_to_send; i++) {
            bufs[i] = tx.pop();
        }
        uint16_t num_tx = rte::eth_tx_burst(port_id, 0, bufs, i);
        if (num_tx < i) {
            for (uint16_t j=0; j<i-num_tx; j++) {
                rte::pktmbuf_free(bufs[num_tx+j]);
            }
        }
        num_tx_sum += num_tx;
    }

    tx_packets += num_tx_sum;
    return num_tx_sum;
}

void ifnet::stat()
{
    printf("%s: ", name.c_str());
    if (promiscuous_mode) printf("PROMISC ");
    printf("\n");
    printf("\tRX Packets %u Queue %zu\n", rx_packets, rx.size());
    printf("\tTX Packets %u Queue %zu\n", tx_packets, tx.size());
    printf("\n");

}
