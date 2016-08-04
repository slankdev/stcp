
#pragma once

#include <stcp/protocol.h>


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
