

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



#define ETHER_ADDR_LEN 6


#define AF_LINK 0
#define AF_INET 1

typedef uint8_t af_t;

static char* af2str(af_t af)
{
    static char str[8];
    switch (af) {
        case AF_INET:
            strcpy(str, "INET");
            break;
        case AF_LINK:
            strcpy(str, "LINK");
            break;
        default:
            strcpy(str, "UNKNOWN");
            break;
    }
    return str;
}


class if_addr {
    public:
        af_t    family;
        struct {
            union {
                uint8_t data[16];
                struct ether_addr link;
                uint8_t in[4];
            };
        } raw;


    if_addr(af_t af) : family(af) {}
    void init(const void* d, size_t l)
    {
        slankdev::log& log = slankdev::log::instance();
        log.push(af2str(family));

        switch (family) {
            case AF_INET:
            {
                fprintf(stderr, "Not Impl yet\n");
                exit(-1);
                break;
            }
            case AF_LINK:
            {
                if (l != ETHER_ADDR_LEN) {
                    fprintf(stderr, "Invalid Address len\n");
                    exit(-1);
                }
                memcpy(&raw.link, d, l);

                char str[32];
                sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", 
                        raw.link.addr_bytes[0], raw.link.addr_bytes[1], 
                        raw.link.addr_bytes[2], raw.link.addr_bytes[3], 
                        raw.link.addr_bytes[3], raw.link.addr_bytes[5]);
                log.write(slankdev::INFO, "set address %s", str);

                break;
            }
            default:
            {
                fprintf(stderr, "Unknown address family\n");
                exit(-1);
                break;
            }
        }
        log.pop();
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
    static uint16_t rx_ring_size;     /* rx ring size */
    static uint16_t tx_ring_size;     /* tx ring size */
    static uint16_t num_rx_rings;     /* num of rx_rings per port */
    static uint16_t num_tx_rings;     /* num of tx_rings per port */

public:
    slankdev::queue<struct rte_mbuf, myallocator> rx;
    slankdev::queue<struct rte_mbuf, myallocator> tx;
    std::vector<if_addr> addrs;
    std::string name;

public:

    net_device(int n) : port_id(n)
    {
        name = "PORT" + std::to_string(n);
    }
    void init();
    uint16_t io_rx()
    {
        struct rte_mbuf* bufs[BURST_SIZE];
        uint16_t num_rx = rte::eth_rx_burst(port_id, 0, bufs, BURST_SIZE);
        if (unlikely(num_rx == 0)) return 0;

        rx.push(array2llist_mbuf(bufs, num_rx));
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




