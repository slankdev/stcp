

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
#include <stcp/socket.h>


namespace slank {
    

enum {
    STCP_SIOCAARPENT,
    STCP_SIOCDARPENT
};



class arp_module {
private:
    proto_module m;
    std::vector<stcp_arpreq> table;

    void proc_arpreply(struct stcp_arphdr* ah, uint8_t port);
    
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

    void proc_update_arptable(struct stcp_arphdr* ah, uint8_t port);
    void ioctl(uint64_t request, void* arg);

private:
    void ioctl_siocaarpent(stcp_arpreq* req);
    // void ioctl_siocdarpent(stcp_arpreq* req);
};


} /* namespace */
