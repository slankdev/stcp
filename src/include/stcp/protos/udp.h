
#pragma once

#include <stcp/config.h>
#include <stcp/socket.h>
#include <stcp/arch/dpdk/dpdk.h>
#include <stcp/util.h>

#include <vector>
#include <queue>
#include <mutex>


namespace slank {



struct stcp_udp_header {
    uint16_t sport;
    uint16_t dport;
    uint16_t len;
    uint16_t cksum;

    void print() const
    {
        stcp_printf("UDP header \n");
        stcp_printf("+ sport    : %u 0x%04x \n", ntoh16(sport), ntoh16(sport));
        stcp_printf("+ dport    : %u 0x%04x \n", ntoh16(dport), ntoh16(dport));
        stcp_printf("+ len      : %u 0x%08x \n", ntoh16(len  ), ntoh16(len  ));
        stcp_printf("+ cksum    : 0x%04x \n"   , ntoh16(cksum)  );
    }
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



class stcp_udp_sock {
    friend class udp_module;
private:
    udp_sock_state state;  /* state of the socket   */
    queue_TS<stcp_udp_sockdata> rxq; /* receive queue         */
    queue_TS<stcp_udp_sockdata> txq; /* transmission queue    */
    uint16_t port;         /* stored as NwByteOrder */
    stcp_in_addr addr;     /* binded address        */
    void proc();

public:
    stcp_udp_sock() : state(unbind) {}
    bool operator==(const stcp_udp_sock& rhs) const { return port==rhs.port; }
    bool operator!=(const stcp_udp_sock& rhs) const { return !(*this==rhs); }

public: /* for Users Operation */
    void sendto(mbuf* msg, const stcp_sockaddr_in* src);
    mbuf* recvfrom(stcp_sockaddr_in* src);
    void bind(const stcp_sockaddr_in* a);
};


class udp_module {
    friend class core;
private:
    std::vector<stcp_udp_sock*> socks;

public:
    udp_module() {}
    void rx_push(mbuf* msg, stcp_sockaddr_in* src);
    void tx_push(mbuf* msg, const stcp_sockaddr_in* dst, uint16_t srcp);
    void print_stat() const;
    void proc();
};


} /* namespace slank */
