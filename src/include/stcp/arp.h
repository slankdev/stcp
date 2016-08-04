

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


struct arpentry {
    uint8_t port;
    struct ip_addr    ip;
    struct ether_addr mac;

    arpentry(ip_addr i, ether_addr e, uint8_t p)
    {
        ip = i;
        mac = e;
        port = p;
    }
};

struct arphdr {
    uint16_t          hwtype;
    uint16_t          ptype;
    uint8_t           hwlen;
    uint8_t           plen;
    uint16_t          operation;
    struct ether_addr hwsrc;
    struct ip_addr    psrc;
    struct ether_addr hwdst;
    struct ip_addr    pdst;
};



class arp_module {
private:
    proto_module m;
    std::vector<arpentry> table;

public:
    arp_module() { m.name = "ARP"; }
    void init() {m.init();}
    void rx_push(struct rte_mbuf* msg){m.rx_push(msg);}
    void tx_push(struct rte_mbuf* msg){m.tx_push(msg);}
    struct rte_mbuf* rx_pop() {return m.rx_pop();}
    struct rte_mbuf* tx_pop() {return m.tx_pop();}
    void drop(struct rte_mbuf* msg) {m.drop(msg);}
    void add_cache(struct ip_addr& ip, struct ether_addr& mac, ifnet* iface);

    void stat();
    void proc();
    void update_table(arpentry newent);
};


