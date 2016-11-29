

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <queue>

#include <stcp/config.h>
#include <stcp/socket.h>


namespace slank {



enum arpop : uint16_t {
    ARPOP_REQUEST    = 1,
    ARPOP_REPLY      = 2,
    ARPOP_REVREQUEST = 3,
    ARPOP_REVREPLY   = 4,
};


struct stcp_arphdr {
    uint16_t            hwtype;
    uint16_t            ptype;
    uint8_t             hwlen;
    uint8_t             plen;
    uint16_t            operation;
    struct stcp_ether_addr   hwsrc;
    struct stcp_in_addr psrc;
    struct stcp_ether_addr   hwdst;
    struct stcp_in_addr pdst;
};



struct stcp_arpreq {
    struct stcp_sockaddr arp_pa;		/* Protocol address.  */
    struct stcp_sockaddr arp_ha;		/* Hardware address.  */
    uint8_t              arp_ifindex;

public:
    stcp_arpreq() : arp_pa(STCP_AF_INET), arp_ha(STCP_AF_LINK) {}
    stcp_arpreq(const stcp_sockaddr* pa, const stcp_sockaddr* ha, uint8_t index) :
        arp_pa(*pa), arp_ha(*ha), arp_ifindex(index) {}
    bool operator==(const stcp_arpreq& rhs) const
    {
        bool r1 = (arp_pa      == rhs.arp_pa     );
        bool r2 = (arp_ha      == rhs.arp_ha     );
        bool r3 = (arp_ifindex == rhs.arp_ifindex);
        return r1 && r2 && r3;
    }
    bool operator!=(const stcp_arpreq& rhs) const
    {
        return !(*this==rhs);
    }
    stcp_arpreq& operator=(const stcp_arpreq& rhs)
    {
        arp_pa      = rhs.arp_pa;
        arp_ha      = rhs.arp_ha;
        arp_ifindex = rhs.arp_ifindex;
        return *this;
    }
};




struct wait_ent {
    uint8_t port;
    mbuf* msg;
    stcp_sockaddr dst;
    wait_ent(uint8_t p, mbuf* m, stcp_sockaddr d) :
        port(p), msg(m), dst(d) {}
};



class arp_module {
private:
    bool use_dynamic_arp;
private:
    size_t rx_cnt;
    size_t tx_cnt;
    std::vector<stcp_arpreq> table;

public:
    std::queue<wait_ent> arpresolv_wait_queue;

public:
    arp_module() : use_dynamic_arp(true), rx_cnt(0), tx_cnt(0) {}
    void rx_push(mbuf* msg);
    void tx_push(mbuf* msg);

    /*
     * TODO reimplement smartly
     */
    bool arp_resolv(uint8_t port, const stcp_sockaddr *dst,
            stcp_ether_addr* dsten, bool checkcacheonly=false);
    void arp_request(uint8_t port, const stcp_in_addr* tip);
    void ioctl(uint64_t request, void* arg);
    void print_stat() const;

private:
    void ioctl_siocaarpent(stcp_arpreq* req);
    void ioctl_siocdarpent(stcp_arpreq* req);
    void ioctl_siocgarpent(std::vector<stcp_arpreq>** tbl);
    void ioctl_siocsdarp(const bool* b);
    void ioctl_siocgdarp(bool* b);
};


} /* namespace */
