


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

#include <slankdev/queue.h>
#include <slankdev/log.h>
#include <slankdev/singleton.h>
#include <stcp/rte.h>
#include <stcp/net_device.h>









class dpdk : public slankdev::singleton<dpdk> {
    friend slankdev::singleton<dpdk>;
private:
    static uint32_t num_mbufs;        /* num of mbuf that allocated in a mempool */
    static uint32_t mbuf_cache_size;  /* packet cache size in each mbufs */
    struct rte_mempool* mempool;

    dpdk() : mempool(nullptr) {}
    ~dpdk() {}                  

public:
    std::vector<net_device> devices;

    void init(int argc, char** argv)
    {
        slankdev::log& log = slankdev::log::instance();
        log.push("DPDK");

        rte::eth_dev_init(argc, argv);
        if (rte::eth_dev_count() < 1) {
            throw rte::exception("num of devices is less than 1");
        }
        log.write(slankdev::INFO, "create memry pool");
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

        log.pop();
    }
    struct rte_mempool* get_mempool()
    {
        return mempool;
    }
};



