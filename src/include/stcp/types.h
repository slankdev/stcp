

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <rte_mbuf.h> // for struct ether_header






namespace slank {



struct stcp_ether_header {
    struct ether_addr dst;
    struct ether_addr src;
    uint16_t type;
};





} /* namespace */
