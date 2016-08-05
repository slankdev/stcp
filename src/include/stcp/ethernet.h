
#pragma once


#include <stcp/rte.h>


namespace slank {
    


// struct ether_addr {
//     uint8_t addr_bytes[6];
//
//     bool operator=(const struct mac_addr& rhs) 
//     {
//         for (int i=0; i<6; i++) {
//             if (addr_bytes[i] != rhs.addr_byte[i]) return false;
//         }
//         return true;
//     }
// };


struct ether_header {
    struct ether_addr dst;
    struct ether_addr src;
    uint16_t type;
};


} /* namespace */
