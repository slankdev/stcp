

#pragma once

#include <stcp/rte.h>
#include <slankdev/queue.h>
#include <slankdev/util.h>
#include <slankdev/exception.h>
#include <slankdev/system.h>


namespace slank {
 
using exception = slankdev::exception;
using eth_conf = struct rte_eth_conf;
using mbuf = struct rte_mbuf;
using slankdev::clear_screen;
using pkt_queue = slankdev::queue<struct rte_mbuf, rte_mbuf_allocator>;

// struct tag;
// using log = slankdev::log<tag>;

} /* namespace */
