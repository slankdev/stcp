
#pragma once

#include <stcp/dpdk.h>
#include <stcp/protocol.h>






class ip_module {
private:
    proto_module m;

public:

    ip_module() { m.name = "IP"; }
    void init()
    {
        m.init();
    }

    void rx_push(struct rte_mbuf* msg){m.rx_push(msg);}
    void tx_push(struct rte_mbuf* msg){m.tx_push(msg);}
    struct rte_mbuf* rx_pop() {return m.rx_pop();}
    struct rte_mbuf* tx_pop() {return m.tx_pop();}
    void drop(struct rte_mbuf* msg) {m.drop(msg);}
    void proc() {m.proc();}
    void stat() {m.stat();}
};
