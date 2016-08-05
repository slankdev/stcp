
#pragma once

#include <stcp/config.h>
#include <stcp/rte.h> // struct ether_addr
#include <stcp/types.h>

#include <stdint.h>
#include <stddef.h>

#define ETHER_ADDR_LEN 6


namespace slank {
    


const char* af2str(sa_family af);


class ifaddr {
public:
    sa_family family;
    struct {
        union {
            uint8_t data[16];
            struct ether_addr link;
            struct ip_addr in;
        };
    } raw;

    ifaddr(sa_family af) : family(af) {}
    void init(const void* d, size_t l);
};




} /* namespace */
