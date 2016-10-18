
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
    uint16_t sport;
    uint16_t dport;
    uint16_t len;
    stcp_sockaddr addr;

    stcp_udp_sockdata(mbuf* m, stcp_sockaddr a, uint16_t src, uint16_t dst) 
        : msg(m), 
        sport(rte::bswap16(src)),                     /* Specifies as NetworkByteOrder */ 
        dport(rte::bswap16(dst)),                     /* Specifies as NetworkByteOrder */ 
        len  (rte::bswap16(rte::pktmbuf_pkt_len(m))), /* Specifies as NetworkByteOrder */ 
        addr(a) {}
};


using udp_sock_queue = std::queue<stcp_udp_sockdata>;


struct stcp_udp_sock {
    friend class core;
    bool binded;
    udp_sock_queue rxq;
    udp_sock_queue txq;
    stcp_sockaddr_in addr;
public:

    stcp_udp_sock() : binded(false) {}
    void sendto(mbuf* msg, const stcp_sockaddr* src);
    bool recvfrom(mbuf** msg, stcp_sockaddr* src);
    void bind(const stcp_sockaddr_in* a)
    {
        addr = *a;
        binded = true;
    }
    bool readable() const
    {
        return rxq.size() > 0;
    }
    bool operator==(const stcp_udp_sock& rhs) const
    {
        return addr.sin_port == rhs.addr.sin_port; 
    }
    bool operator!=(const stcp_udp_sock& rhs) const
    {
        return !(*this==rhs);
    }
};

class udp_module {
    friend class core;
private:
    size_t rx_cnt;
    size_t tx_cnt;

    std::vector<stcp_udp_sock> socks;

public:
    udp_module() : rx_cnt(0), tx_cnt(0) {}
    void rx_push(mbuf* msg, const stcp_sockaddr* src);
    void tx_push(mbuf* msg, const stcp_sockaddr* dst, 
            uint16_t srcp, uint16_t dstp);

    stcp_udp_sock& socket()
    {
        stcp_udp_sock sock;
        socks.push_back(sock);
        return socks[socks.size()-1];
    }
};


} /* namespace slank */
