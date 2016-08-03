

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string>

#include <stcp/rte.h>
#include <stcp/config.h>


class proto_module {
protected:
    pkt_queue rx;
    pkt_queue tx;
    uint16_t rx_packets;
    uint16_t tx_packets;
    uint16_t drop_packets;


public:
    std::string name;

    proto_module() : 
        rx_packets(0), 
        tx_packets(0),
        drop_packets(0) {}
    virtual void init() {}
    virtual void proc() {}
    virtual void stat()
    {
        printf("%s module \n", name.c_str());
        printf("\tRX Packets %u Queue %zu\n", rx_packets, rx.size());
        printf("\tTX Packets %u Queue %zu\n", tx_packets, tx.size());
        printf("\tDrop Packets %u\n", drop_packets);
    }

    void rx_push(struct rte_mbuf* msg) { rx.push(msg); rx_packets++; }
    void tx_push(struct rte_mbuf* msg) { tx.push(msg); tx_packets++; }
    struct rte_mbuf* rx_pop() { return rx.pop(); }
    struct rte_mbuf* tx_pop() { return tx.pop(); }
    void drop(struct rte_mbuf* msg) { rte::pktmbuf_free(msg); drop_packets++; }
    size_t rx_size() { return rx.size(); }
    size_t tx_size() { return tx.size(); }
};




class ip_module {
private:
    proto_module m;
public:
    ip_module() { m.name = "IP"; }
    void init() {m.init();}
    void rx_push(struct rte_mbuf* msg){m.rx_push(msg);}
    void tx_push(struct rte_mbuf* msg){m.tx_push(msg);}
    struct rte_mbuf* rx_pop() {return m.rx_pop();}
    struct rte_mbuf* tx_pop() {return m.tx_pop();}
    void drop(struct rte_mbuf* msg) {m.drop(msg);}
    void proc() {m.proc();}
    void stat() {m.stat();}
};

