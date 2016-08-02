


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
#include <vector>

#include <stcp/rte.h>
#include <stcp/net_device.h>
#include <stcp/config.h>







class dpdk {
private:
    static bool inited;
    static uint32_t num_mbufs;        /* num of mbuf that allocated in a mempool */
    static uint32_t mbuf_cache_size;  /* packet cache size in each mbufs */
    struct rte_mempool* mempool;

    dpdk() : mempool(nullptr) {}
    ~dpdk() {}                  

public:
    std::vector<net_device> devices;

    static dpdk& instance()
    {
        static dpdk d;
        return d;
    }

    void init(int argc, char** argv)
    {
        log& log = log::instance();
        log.push("DPDK");

        if (inited)
            throw rte::exception("try to reinit dpdk");

        rte::eth_dev_init(argc, argv);
        if (rte::eth_dev_count() < 1) {
            throw rte::exception("num of devices is less than 1");
        }
        log.write(INFO, "create memry pool");
        mempool = rte::pktmbuf_pool_create(
                "SLANK", 
                num_mbufs * rte::eth_dev_count(), 
                mbuf_cache_size, 
                0, 
                RTE_MBUF_DEFAULT_BUF_SIZE, 
                rte::socket_id()
                );

        for (size_t port=0; port<rte::eth_dev_count(); port++) {
            net_device dev(port);
            dev.init();
            devices.push_back(dev);
        }
        inited = true;

        log.pop();
    }
    struct rte_mempool* get_mempool()
    {
        return mempool;
    }
};
