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

#include <slankdev.h>
#include <stcp/rte.h>
#include <stcp/dpdk.h>






namespace dpdk {

struct rte_mbuf* array2llist_mbuf(struct rte_mbuf** bufs, size_t num_bufs);


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




class myallocator {
    public:
        void deallocate(struct rte_mbuf* ptr)
        {
            rte::pktmbuf_free(ptr);
        }
};

#define BURST_SIZE 32
class net_device {
    private:
        int port_id;

    public:
        slankdev::queue<struct rte_mbuf, myallocator> rx;
        slankdev::queue<struct rte_mbuf, myallocator> tx;

        std::string name;
        std::vector<if_addr> addrs;

        net_device(int n) : port_id(n)
        {
            name = "port" + std::to_string(n);
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

        uint16_t io_rx()
        {
            struct rte_mbuf* bufs[BURST_SIZE];
            uint16_t num_rx = rte::eth_rx_burst(port_id, 0, bufs, BURST_SIZE);
            if (unlikely(num_rx == 0)) return 0;

            rx.push(dpdk::array2llist_mbuf(bufs, num_rx));
            printf("%s: recv %u packets\n", name.c_str(), num_rx);
            return num_rx;
        }
        uint16_t io_tx(size_t num_request_to_send)
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

            printf("%s: sent %u packets\n", name.c_str(), num_tx_sum);
            return num_tx_sum;
        }
    
        void stat()
        {
            printf("rx/tx = %zd/%zd \n", rx.size(), tx.size());
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
    std::vector<net_device> devices;
    static core& instance();              /* for singleton */
    void init(int argc, char** argv);     /* init rte_eal and ports */

    struct rte_mempool* get_mempool()
    {
        return mempool;
    }
};







    
} /* namespace dpdk */





