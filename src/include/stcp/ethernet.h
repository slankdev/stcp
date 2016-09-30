
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include <stcp/rte.h>
#include <stcp/protocol.h>
#include <stcp/config.h>
#include <stcp/socket.h>
#include <stcp/mbuf.h>


namespace slank {


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
    friend class core;
    proto_module m;
    size_t rx_cnt;
    size_t tx_cnt;

public:
    ether_module() : rx_cnt(0), tx_cnt(0) {}

    void rx_push(mbuf* msg);
    void tx_push(uint8_t port, mbuf* msg, const stcp_sockaddr* dst);
    void proc();

public:

    /*
     * XXX TODO reimpelemnt about below
     * this function argument, dst should be destination macaddress.
     * but this function use it as destination ipaddress.
     * ether_module::sendto() should work as Ethernet ctrl.
     * Now implementation, this func works as ip_module.
     */
    void sendto(const void* buf, size_t bufsize, const stcp_sockaddr* dst);

};


} /* namespace slank */
