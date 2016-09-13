

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <rte_mbuf.h> // for struct ether_header






namespace slank {


struct stcp_in_addr {
    uint8_t addr_bytes[4];

    bool operator==(const struct stcp_in_addr& rhs) 
    {
        for (int i=0; i<4; i++) {
            if (addr_bytes[i] != rhs.addr_bytes[i]) return false;
        }
        return true;
    }
};

struct stcp_ether_header {
    struct ether_addr dst;
    struct ether_addr src;
    uint16_t type;
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




} /* namespace */
