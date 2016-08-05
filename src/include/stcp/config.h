

#pragma once

#include <stcp/rte.h>
// #include <slankdev/log.h>
#include <slankdev/queue.h>
#include <slankdev/util.h>
#include <slankdev/exception.h>
#include <slankdev/system.h>


namespace slank {
    

using slankdev::clear_screen;
using pkt_queue = slankdev::queue<struct rte_mbuf, rte_mbuf_allocator>;

// struct tag;
// using log = slankdev::log<tag>;

} /* namespace */
