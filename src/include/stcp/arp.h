

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include <stcp/protocol.h>
#include <stcp/ip.h>


//
// class arpentry {
//     ipaddress ip;
//     macaddress mac;
// };



class arp_module {
private:
    proto_module m;



public:
    arp_module() { m.name = "ARP"; }
    void init() {m.init();}
    void rx_push(struct rte_mbuf* msg){m.rx_push(msg);}
    void tx_push(struct rte_mbuf* msg){m.tx_push(msg);}
    struct rte_mbuf* rx_pop() {return m.rx_pop();}
    struct rte_mbuf* tx_pop() {return m.tx_pop();}
    void drop(struct rte_mbuf* msg) {m.drop(msg);}
    void proc() {m.proc();}
    void stat() {m.stat();}
};

