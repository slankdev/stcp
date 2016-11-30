
#pragma once

#include <stcp/arch/dpdk/rte.h>

namespace slank {

inline size_t eth_dev_count()
{
    return rte::eth_dev_count();
}

inline uint16_t cpu_socket_id()
{
    return rte::socket_id();
}


} /* namespace slank */
