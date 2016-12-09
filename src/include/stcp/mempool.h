
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stcp/config.h>

namespace slank {

inline mempool* pool_create(
        const char* name,
        unsigned    n,
        unsigned    cache_size,
        uint16_t    priv_size,
        uint16_t    data_room_size,
        int         socket_id)
{
    return rte::pktmbuf_pool_create(name, n, cache_size,
            priv_size, data_room_size, socket_id);
}


inline void pool_dump(rte_mempool* mp)
{
    DPRINT("Mempool\n");
    DPRINT(" name         : %s \n", mp->name);
    DPRINT(" size         : %u \n", mp->size);
    DPRINT(" cache        : %u \n", mp->cache_size);
    DPRINT(" nb_mem_chunks: %u \n", mp->nb_mem_chunks);

    DPRINT(" avail/in_use : %u/%u \n",
        rte_mempool_avail_count(mp), rte_mempool_in_use_count(mp));
}


} /* namespace slank */
