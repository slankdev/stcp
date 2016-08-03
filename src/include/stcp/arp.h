

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include <stcp/protocol.h>
#include <stcp/ethernet.h>
#include <stcp/ip.h>
#include <stcp/config.h>
#include <stcp/ifnet.h>


//
class arpentry {
    ipaddr_t ip;
    struct ether_addr mac;
    ifnet* iface;
};



struct arphdr {
    uint16_t          hwtype;
    uint16_t          ptype;
    uint8_t           hwlen;
    uint8_t           plen;
    uint16_t          operation;
    struct ether_addr hwsrc;
    uint32_t          psrc;
    struct ether_addr hwdst;
    uint32_t          pdst;
};


enum class arp_op {
    request = 1,
    replay,
};




class arp_module {
private:
    proto_module m;

    pkt_queue wait_queue;


public:
    arp_module() { m.name = "ARP"; }
    void init() {m.init();}
    void rx_push(struct rte_mbuf* msg){m.rx_push(msg);}
    void tx_push(struct rte_mbuf* msg){m.tx_push(msg);}
    struct rte_mbuf* rx_pop() {return m.rx_pop();}
    struct rte_mbuf* tx_pop() {return m.tx_pop();}
    void drop(struct rte_mbuf* msg) {m.drop(msg);}

    void stat() 
    {
        m.stat();
        printf("\tWait Packets %zd\n", wait_queue.size());
    }
    void proc();
};

