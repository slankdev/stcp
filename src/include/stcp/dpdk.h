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
#include <stcp/dpdk.h>






namespace dpdk {


enum address_family {
    link,
    inet
};
class if_addr {
    public:
        enum address_family af;
        uint8_t data[16];

        if_addr(address_family a, const void* raw, size_t rawlen)
        {
            if (rawlen > sizeof(data))
                exit(-1);

            af = a;
            memcpy(data, raw, rawlen);
        }

    
};





class net_device {
    public:
        std::string name;
        std::vector<if_addr> addrs;

        net_device(std::string n) : name(n)
        {
            printf("[net_device:%s] init \n", name.c_str());
        }
        void set_hw_addr(struct ether_addr* addr)
        {
            if_addr ifaddr(link, addr, sizeof(struct ether_addr));
            addrs.push_back(ifaddr);

            printf("[net_device:%s] set address ", name.c_str());
            for (int i=0; i<6; i++) {
                printf("%02x", addr->addr_bytes[i]);
                if (i==5) printf("\n");
                else      printf(":");
            }
        }
};




class core {
private:
    static uint32_t num_mbufs;        /* num of mbuf that allocated in a mempool */
    static uint32_t mbuf_cache_size;  /* packet cache size in each mbufs */
    static uint16_t rx_ring_size;     /* rx ring size */
    static uint16_t tx_ring_size;     /* tx ring size */
    static uint16_t num_rx_rings;     /* num of rx_rings per port */
    static uint16_t num_tx_rings;     /* num of tx_rings per port */

    struct rte_mempool* mempool;
    void port_init(uint8_t port);

private:                                  /* for singleton */
    core();                               /* for singleton */
    core(const core&) = delete;           /* for singleton */
    core& operator=(const core&) =delete; /* for singleton */
    ~core();                              /* for singleton */

public:
    static core& instance();              /* for singleton */

    void init(int argc, char** argv);     /* init rte_eal and ports */
    uint16_t io_rx(uint16_t port, struct rte_mbuf** bufs, size_t num_bufs);
    uint16_t io_tx(uint16_t port, struct rte_mbuf** bufs, size_t num_bufs);

    std::vector<net_device> devices;
};


struct rte_mbuf* array2llist_mbuf(struct rte_mbuf** bufs, size_t num_bufs);





    
} /* namespace dpdk */





