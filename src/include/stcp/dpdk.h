


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


class core {
private:
    static uint32_t num_mbufs;        /* num of mbuf that allocated in a mempool */
    static uint32_t mbuf_cache_size;  /* packet cache size in each mbufs */
    static uint16_t rx_ring_size;     /* rx ring size */
    static uint16_t tx_ring_size;     /* tx ring size */
    static uint16_t num_rx_rings;     /* num of rx_rings per port */
    static uint16_t num_tx_rings;     /* num of tx_rings per port */

    struct rte_mempool* mempool;
    void port_init(uint8_t port)
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


private:                                  /* for singleton */
    core() : mempool(nullptr) {}          /* for singleton */
    core(const core&) = delete;           /* for singleton */
    core& operator=(const core&) =delete; /* for singleton */
    ~core() {}                            /* for singleton */

public:
    std::vector<net_device> devices;
    static core& instance()               /* for singleton */
    {
        static core instance;
        return instance;
    }
    void init(int argc, char** argv)      /* init rte_eal and ports */
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

    struct rte_mempool* get_mempool()
    {
        return mempool;
    }
};

uint16_t core::rx_ring_size = 128;
uint16_t core::tx_ring_size = 512;
uint32_t core::num_mbufs = 8192;
uint32_t core::mbuf_cache_size = 250;
uint16_t core::num_rx_rings = 1;
uint16_t core::num_tx_rings = 1;






    
} /* namespace dpdk */





