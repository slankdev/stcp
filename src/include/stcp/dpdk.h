
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
    void init(int argc, char** argv);
    struct rte_mempool* get_mempool() { return mempool; }
};

} /* namespace */
