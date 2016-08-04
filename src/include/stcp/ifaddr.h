
#pragma once

#include <stcp/rte.h> // struct ether_addr
#include <stdint.h>
#include <stddef.h>

#define ETHER_ADDR_LEN 6

#define STCP_AF_LINK 0
#define STCP_AF_INET 2

typedef uint8_t af_t;

class ifaddr {
public:
    af_t family;
    struct {
        union {
            uint8_t data[16];
            struct ether_addr link;
            uint8_t in[4];
        };
    } raw;

    ifaddr(af_t af) : family(af) {}
    void init(const void* d, size_t l);
};




