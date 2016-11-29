
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include <stcp/rte.h>
#include <stcp/config.h>
#include <stcp/socket.h>
#include <stcp/mbuf.h>


namespace slank {


struct stcp_ether_header {
    struct stcp_ether_addr dst;
    struct stcp_ether_addr src;
    uint16_t type;
};

enum stcp_ether_type : uint16_t {
    ETHERTYPE_IP     = 0x0800,
    ETHERTYPE_ARP    = 0x0806,
    ETHERTYPE_REVARP = 0x8035,
};


class ether_module {
private:
    size_t rx_cnt;
    size_t tx_cnt;

public:
    ether_module() : rx_cnt(0), tx_cnt(0) {}

    void rx_push(mbuf* msg);
    void tx_push(uint8_t port, mbuf* msg, const stcp_sockaddr* dst);
    void proc();
    void print_stat() const;
};


} /* namespace slank */
