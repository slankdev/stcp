

#pragma once

#include <stcp/rte.h>
#include <stcp/exception.h>
#include <slankdev/queue.h>
#include <slankdev/util.h>
#include <slankdev/exception.h>
#include <slankdev/system.h>
#include <stcp/log.h>
#include <queue>
#include <stdio.h>


namespace slank {

#define DEBUG(...) \
    printf("%s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__) 
 
using eth_conf = struct rte_eth_conf;
using mbuf = struct rte_mbuf;
using slankdev::clear_screen;
// using pkt_queue = slankdev::queue<struct rte_mbuf, rte_mbuf_allocator>;
using pkt_queue = std::queue<struct rte_mbuf*>;


} /* namespace */
