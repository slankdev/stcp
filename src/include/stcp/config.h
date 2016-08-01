

#pragma once

#include <stcp/rte.h>


#include <slankdev/log.h>
#include <slankdev/queue.h>
#include <slankdev/util.h>

using slankdev::clear_screen;

struct tag1;
using log = slankdev::log<tag1>;
using pkt_queue = slankdev::queue<struct rte_mbuf, myallocator>;
