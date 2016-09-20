

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include <stcp/protocol.h>
#include <stcp/config.h>
#include <stcp/socket.h>


namespace slank {
    

enum {
    STCP_SIOCAARPENT,
    STCP_SIOCDARPENT,
    STCP_SIOCGARPENT,
};

enum stcp_arpop : uint16_t {
    STCP_ARPOP_REQUEST    = 1,
    STCP_ARPOP_REPLY      = 2,  
    STCP_ARPOP_REVREQUEST = 3,
    STCP_ARPOP_REVREPLY   = 4,
};


struct stcp_arphdr {
    uint16_t            hwtype;
    uint16_t            ptype;
    uint8_t             hwlen;
    uint8_t             plen;
    uint16_t            operation;
    struct ether_addr   hwsrc;
    struct stcp_in_addr psrc;
    struct ether_addr   hwdst;
    struct stcp_in_addr pdst;
};



struct stcp_arpreq {
    struct stcp_sockaddr arp_pa;		/* Protocol address.  */
    struct stcp_sockaddr arp_ha;		/* Hardware address.  */
    uint8_t              arp_ifindex;

public:
    stcp_arpreq() {}
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


class arp_module {
private:
    bool use_dynamic_arp;
private:
    proto_module m;
    std::vector<stcp_arpreq> table;

    void proc_arpreply(struct stcp_arphdr* ah, uint8_t port);
    
public:
    arp_module() : use_dynamic_arp(false) { m.name = "ARP"; }
    void init() {m.init();}
    void rx_push(mbuf* msg){m.rx_push(msg);}
    void tx_push(mbuf* msg){m.tx_push(msg);}
    mbuf* rx_pop() {return m.rx_pop();}
    mbuf* tx_pop() {return m.tx_pop();}
    void drop(mbuf* msg) {m.drop(msg);}

    void stat();
    void proc();

    void ioctl(uint64_t request, void* arg);
    void arp_resolv(uint8_t port, const stcp_sockaddr *dst, uint8_t* dsten);

private:
    void ioctl_siocaarpent(stcp_arpreq* req);
    void ioctl_siocdarpent(stcp_arpreq* req);
    void ioctl_siocgarpent(std::vector<stcp_arpreq>** tbl);
};


} /* namespace */
