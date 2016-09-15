
#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include <vector>
#include <string>

#include <stcp/rte.h>
#include <stcp/ifnet.h>
#include <stcp/config.h>



namespace slank {



class dpdk {
private:
    static uint32_t num_mbufs;        /* num of mbuf that allocated in a mempool */
    static uint32_t mbuf_cache_size;  /* packet cache size in each mbufs         */
    static std::string mp_name;       /* memory pool name                        */
    struct rte_mempool* mempool;
    dpdk() : mempool(nullptr) {}
    ~dpdk() {}                  

public:
    std::vector<ifnet> devices;
    static dpdk& instance()
    {
        static dpdk d;
        return d;
    }
    void init(int argc, char** argv)
    {

        rte::eth_dev_init(argc, argv);
        mempool = rte::pktmbuf_pool_create(
                mp_name.c_str(), 
                num_mbufs * rte::eth_dev_count(), 
                mbuf_cache_size, 
                0, 
                RTE_MBUF_DEFAULT_BUF_SIZE, 
                rte::socket_id()
                );

        for (size_t port=0; port<rte::eth_dev_count(); port++) {
            ifnet dev(port);
            dev.init();
            devices.push_back(dev);
        }
    }

    struct rte_mempool* get_mempool() { return mempool; }
};

} /* namespace */
