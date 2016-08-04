
#pragma once


#include <stcp/rte.h>



struct ether_header {
    struct ether_addr dst;
    struct ether_addr src;
    uint16_t type;
};
