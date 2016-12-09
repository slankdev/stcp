

#pragma once

#include <stcp/config.h>
#include <stcp/socket.h>
#include <stcp/arch/dpdk/dpdk.h>
#include <stcp/stcp.h>
#include <stcp/util.h>
#include <stcp/protos/tcp_var.h>
#include <stcp/protos/tcp_socket.h>
#include <vector>




namespace slank {





class tcp_module {
    friend class core;
    friend class stcp_tcp_sock;
private:
    static size_t mss;
    mempool* mp;
    std::vector<stcp_tcp_sock> socks;

public:
    tcp_module() : mp(nullptr), socks(5) {}
    void init();
    void rx_push(mbuf* msg, stcp_sockaddr_in* src);
    void tx_push(mbuf* msg, const stcp_sockaddr_in* dst);

    void proc();
    void print_stat() const;
};




} /* namespace slank */
