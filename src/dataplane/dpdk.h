#pragma once

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

#include <rte_config.h>
#include <rte_version.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_hexdump.h>





namespace dpdk {


class core {
private:
    static uint16_t rx_ring_size;
    static uint16_t tx_ring_size;
    static uint32_t num_mbufs;
    static uint32_t mbuf_cache_size;

    struct rte_mempool* mempool;
    void port_init(uint8_t port);
public:

    core();
    void init(int argc, char** argv);

    size_t num_ports();
    uint16_t io_rx(uint16_t port, struct rte_mbuf** bufs, size_t num_bufs);
    uint16_t io_tx(uint16_t port, struct rte_mbuf** bufs, size_t num_bufs);
};







class conf {
public:
    struct rte_eth_conf port_conf;

    conf()
    {
        init_port_conf();
    }
    ~conf() {}

    void init_port_conf()
    {
        struct rte_eth_rxmode rxmode;
        memset(&rxmode, 0, sizeof rxmode);
        rxmode.max_rx_pkt_len = ETHER_MAX_LEN;

        memset(&port_conf, 0, sizeof(port_conf));
        port_conf.rxmode = rxmode;
    }
};

    
} /* namespace dpdk */





