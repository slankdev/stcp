
#pragma once

#include <stcp/config.h>
#include <stcp/rte.h> // struct ether_addr

#include <stdint.h>
#include <stddef.h>

#define ETHER_ADDR_LEN 6

enum af_t {
    af_link=0,
    af_inet=2,
};

namespace slank {
    

// using af_t=uint8_t;

struct ip_addr {
    uint8_t addr_bytes[4];

    bool operator==(const struct ip_addr& rhs) 
    {
        for (int i=0; i<4; i++) {
            if (addr_bytes[i] != rhs.addr_bytes[i]) return false;
        }
        return true;
    }
};


const char* af2str(af_t af);



class ifaddr {
public:
    af_t family;
    struct {
        union {
            uint8_t data[16];
            struct ether_addr link;
            struct ip_addr in;
        };
    } raw;

    ifaddr(af_t af) : family(af) {}
    void init(const void* d, size_t l);
};




} /* namespace */
