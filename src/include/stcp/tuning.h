
#pragma once
#include <stdint.h>
#include <stddef.h>

namespace slank {


#define ST_TCPMODULE_MEMPOOL_NSEG 8192
#define ST_IPMODULE_IND_MEMPOOL_NSEG  8192
#define ST_IPMODULE_DIR_MEMPOOL_NSEG  8192
#define ST_ARPMODULE_MEMPOOL_NSEG 8192
#define ST_DATAPLANE_MEMPOOL_NSEG 8192

// #define ETHER_MAX_LEN
// #define ETHER_MIN_LEN
// #define ETHER_CRC_LEN
#define ST_ETHER_MTU 1500

#define ST_NB_TCPSOCKET_ALLOC 5


} /* namespace slank */
