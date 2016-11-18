

#pragma once

#include <stcp/rte.h>
#include <stcp/exception.h>
#include <stcp/log.h>
#include <queue>
#include <stdio.h>


namespace slank {

#define DEBUG(...) \
    printf("%-10s:%4d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__)

using eth_conf = struct rte_eth_conf;
using mbuf = struct rte_mbuf;
using pkt_queue = std::queue<struct rte_mbuf*>;


} /* namespace */
