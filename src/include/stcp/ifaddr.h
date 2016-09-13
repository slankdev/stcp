
#pragma once

#include <stcp/config.h>
#include <stcp/rte.h> // struct ether_addr
#include <stcp/types.h>
#include <stcp/socket.h>

#include <stdint.h>
#include <stddef.h>



enum {
    STCP_ETHER_ADDR_LEN = 6
};


namespace slank {
    


const char* af2str(stcp_sa_family af);

struct stcp_ip_addr stcp_inet_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4);
struct stcp_ip_addr stcp_inet_addr(const char* fmt);


class ifaddr {
public:
    stcp_sa_family family;
    struct {
        union {
            uint8_t data[16];
            struct ether_addr link;
            struct stcp_ip_addr in;
        };
    } raw;

    ifaddr(stcp_sa_family af) : family(af) {}
    void init(const void* d, size_t l);
};




} /* namespace */
