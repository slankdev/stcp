
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include <stcp/rte.h>
#include <stcp/arp.h>
#include <stcp/ip.h>
#include <stcp/protocol.h>
#include <stcp/config.h>
#include <stcp/socket.h>
#include <stcp/mbuf.h>


namespace slank {

uint16_t get_ether_type(mbuf* msg);

struct stcp_ether_header {
    struct ether_addr dst;
    struct ether_addr src;
    uint16_t type;
};

enum stcp_ether_type : uint16_t {
    STCP_ETHERTYPE_IP     = 0x0800,
    STCP_ETHERTYPE_ARP    = 0x0806, 
    STCP_ETHERTYPE_REVARP = 0x8035,
};


class ether_module {
private:
    arp_module& arp;
    ip_module&  ip;

    proto_module m;

public:
    ether_module(arp_module& a, ip_module& i) : arp(a), ip(i) 
    { m.name = "Ether";}

    void init() {m.init();}
    void rx_push(mbuf* msg){m.rx_push(msg);}
    void tx_push(uint8_t port, mbuf* msg, const stcp_sockaddr* dst);
    mbuf* rx_pop() {return m.rx_pop();}
    mbuf* tx_pop() {return m.tx_pop();}
    void drop(mbuf* msg) {m.drop(msg);}
    void stat() {m.stat();}
    void proc();

public:
    void sendto(const void* buf, size_t bufsize, const stcp_sockaddr* dst);


};


} /* namespace slank */
