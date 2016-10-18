
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

struct stcp_udp_sockdata {
    mbuf* msg;
    stcp_sockaddr addr;

    stcp_udp_sockdata(mbuf* m, stcp_sockaddr a) : msg(m), addr(a) {}
};
using udp_sock_queue = std::queue<stcp_udp_sockdata>;


struct stcp_udp_sock {
    udp_sock_queue rxq;
    udp_sock_queue txq;
    uint16_t port;
};

class udp_module {
private:
    size_t rx_cnt;
    size_t tx_cnt;

    std::vector<stcp_udp_sock> socks;

public:
    udp_module() : rx_cnt(0), tx_cnt(0) {}
    void rx_push(mbuf* msg, const stcp_sockaddr* src);
};


} /* namespace slank */
