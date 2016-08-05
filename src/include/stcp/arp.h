

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include <stcp/protocol.h>
#include <stcp/ip.h>
#include <stcp/config.h>
#include <stcp/ifnet.h>
#include <stcp/dpdk.h>
#include <stcp/types.h>


namespace slank {
    


struct arpentry {
    struct ip_addr    ip;
    struct ether_addr mac;

    arpentry(ip_addr i, ether_addr e)
    {
        ip = i;
        mac = e;
    }
};

struct port_entry {
    std::vector<arpentry> entrys;
    uint8_t port;
};

class arp_module {
private:
    proto_module m;
    std::vector<port_entry> table;

    void proc_arpreply(struct arphdr* ah, uint8_t port);
    void proc_update_arptable(struct arphdr* ah, uint8_t port);
    
public:
    arp_module() { m.name = "ARP"; }
    void init() 
    {
        m.init();
        dpdk& d = dpdk::instance();
        table.resize(d.devices.size());
    }
    void rx_push(struct rte_mbuf* msg){m.rx_push(msg);}
    void tx_push(struct rte_mbuf* msg){m.tx_push(msg);}
    struct rte_mbuf* rx_pop() {return m.rx_pop();}
    struct rte_mbuf* tx_pop() {return m.tx_pop();}
    void drop(struct rte_mbuf* msg) {m.drop(msg);}

    void stat();
    void proc();
};


} /* namespace */
