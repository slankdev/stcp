
#pragma once
#include <stdint.h>
#include <stddef.h>

namespace stcp {


#define ST_TCPMODULE_MEMPOOL_NSEG    8192
#define ST_IPMODULE_IND_MEMPOOL_NSEG 8192
#define ST_IPMODULE_DIR_MEMPOOL_NSEG 8192
#define ST_ARPMODULE_MEMPOOL_NSEG    8192
#define ST_DATAPLANE_MEMPOOL_NSEG    8192

#define ST_TCPMODULE_MP_CACHESIZ    250
#define ST_IPMODULE_IND_MP_CACHESIZ 32
#define ST_IPMODULE_DIR_MP_CACHESIZ 250
#define ST_ARPMODULE_MP_CACHESIZ    250
#define ST_DATAPLANE_MP_CACHESIZ    250

// #define ETHER_MAX_LEN
// #define ETHER_MIN_LEN
// #define ETHER_CRC_LEN
#define ST_ETHER_MTU 1500

#define ST_NB_TCPSOCKET_ALLOC 5
#define ST_MBUF_BUFSIZ 2176 // include headroom

#define ST_IPFRAG_NB_BUCKETS         0x1000
#define ST_IPFRAG_NB_ENT_PER_BUCKET  16
#define ST_IPFRAG_MAX_ENT_PER_BUCKET 0x1000


/*
 * RUNLEV_SPEED:
 *    stdout      -> NONE
 *    stcp_stdout -> stdout.log
 *    stcp_stddbg -> stddbg.log
 *
 * RUNLEV_DEBUG:
 *    stdout      -> ncurses
 *    stcp_stdout -> stdout.log
 *    stcp_stddbg -> stddbg.log
 */
#define RUNLEV_SPEED 1
#define RUNLEV_DEBUG 2
#define ST_RUNLEVEL RUNLEV_SPEED


} /* namespace stcp */
