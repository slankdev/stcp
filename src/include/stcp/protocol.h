

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string>

#include <stcp/rte.h>
#include <stcp/config.h>


namespace slank {
    

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

    void rx_push(mbuf* msg) { rx.push(msg); rx_packets++; }
    void tx_push(mbuf* msg) { tx.push(msg); tx_packets++; }
    mbuf* rx_pop() { return rx.pop(); }
    mbuf* tx_pop() { return tx.pop(); }
    void drop(mbuf* msg) { rte::pktmbuf_free(msg); drop_packets++; }
    size_t rx_size() { return rx.size(); }
    size_t tx_size() { return tx.size(); }
};




} /* namespace */


