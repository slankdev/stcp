
#pragma once

#include <stcp/config.h>
#include <stcp/socket.h>
#include <stcp/dpdk.h>

#include <vector>
#include <queue>
#include <mutex>


namespace slank {



struct stcp_udp_header {
    uint16_t sport;
    uint16_t dport;
    uint16_t len;
    uint16_t cksum;
};

struct stcp_udp_sockdata {
    mbuf* msg;
    stcp_sockaddr_in addr;
    stcp_udp_sockdata(mbuf* m, stcp_sockaddr_in a) : msg(m), addr(a) {}
};

enum udp_sock_state {
    unbind,
    binded,
};

class udp_sock_queue {
    std::queue<stcp_udp_sockdata> queue;
    mutable std::mutex m;
public:
    void push(stcp_udp_sockdata& data)
    {
        std::lock_guard<std::mutex> lg(m);
        queue.push(data);
    }
    stcp_udp_sockdata pop()
    {
        std::lock_guard<std::mutex> lg(m);
        stcp_udp_sockdata d = queue.front();
        queue.pop();
        return d;
    }
    size_t size() const
    {
        std::lock_guard<std::mutex> lg(m);
        return queue.size();
    }
};

class stcp_udp_sock {
    friend class core;
    friend class udp_module;
private:
    udp_sock_state state;  /* state of the socket   */
    udp_sock_queue rxq;    /* receive queue         */
    uint16_t port;         /* stored as NwByteOrder */
    stcp_in_addr addr;     /* binded address        */

public:
    stcp_udp_sock() : state(unbind) {}
    // ~stcp_udp_sock();
    bool operator==(const stcp_udp_sock& rhs) const { return port==rhs.port; }
    bool operator!=(const stcp_udp_sock& rhs) const { return !(*this==rhs); }
    void rx_data_push(stcp_udp_sockdata d) { rxq.push(d); }
    uint16_t get_port() const { return port; }
    size_t get_rxq_size() const { return rxq.size(); }

public: /* for Users Operation */
    void sendto(mbuf* msg, const stcp_sockaddr_in* src) const;
    mbuf* recvfrom(stcp_sockaddr_in* src);
    void bind(const stcp_sockaddr_in* a);
};


class udp_module {
    friend class core;
    friend class stcp_udp_sock; // TODO ERASE
private:
    size_t rx_cnt;
    size_t tx_cnt;
    std::vector<stcp_udp_sock*> socks;

public:
    udp_module() : rx_cnt(0), tx_cnt(0) {}
    void rx_push(mbuf* msg, stcp_sockaddr_in* src);
    void tx_push(mbuf* msg, const stcp_sockaddr_in* dst, uint16_t srcp);
    void print_stat() const;

};


} /* namespace slank */
