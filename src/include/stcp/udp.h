
#pragma once

#include <stcp/protocol.h>
#include <stcp/config.h>
#include <stcp/socket.h>
#include <stcp/dpdk.h>

#include <vector>
#include <queue>


namespace slank {



struct stcp_udp_header {
    uint16_t sport;
    uint16_t dport;
    uint16_t len;
    uint16_t cksum;
};





struct sock_data {
    mbuf* msg;
    stcp_sockaddr addr;

    sock_data(mbuf* m, stcp_sockaddr a) : msg(m), addr(a) {}
};

using sock_queue = std::queue<sock_data>;


struct udp_sock {
    sock_queue rxq;
    sock_queue txq;
    uint16_t port;
};

class udp_module {
private:
    size_t rx_cnt;
    size_t tx_cnt;

    std::vector<udp_sock> socks;

public:
    udp_module() : rx_cnt(0), tx_cnt(0) {}
    void rx_push(mbuf* msg, const stcp_sockaddr* src);
};


} /* namespace slank */
