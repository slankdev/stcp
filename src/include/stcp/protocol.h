

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
    std::string name;

public:
    proto_module() : rx_packets(0), tx_packets(0) {}
    virtual void init() {}
    virtual void proc() {}
    virtual void stat()
    {
        printf("%s module \n", name.c_str());
        printf("\tRX Packets %u Queue %zu\n", rx_packets, rx.size());
        printf("\tTX Packets %u Queue %zu\n", tx_packets, tx.size());
        printf("\n");
    }

    void rx_push(struct rte_mbuf* msg) { rx.push(msg); }
    void tx_push(struct rte_mbuf* msg) { tx.push(msg); }
    struct rte_mbuf* rx_pop() { return rx.pop(); }
    struct rte_mbuf* tx_pop() { return tx.pop(); }
};


class arp_module : public proto_module {
public:
    arp_module() { name = "ARP"; }
};


class ip_module : public proto_module {
public:
    ip_module() { name = "IP"; }
};

