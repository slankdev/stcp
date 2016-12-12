
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stcp/config.h>
#include <stcp/debug.h>

namespace stcp {

inline mempool* pool_create(
        const char* name,
        unsigned    n,
        unsigned    cache_size,
        uint16_t    data_room_size,
        int         socket_id)
{
    return rte::pktmbuf_pool_create(name, n, cache_size,
            0, data_room_size, socket_id);
}


inline void pool_dump(FILE* fp, mempool* mp)
{
    fprintf(fp, "Mempool\n");
    fprintf(fp, " name         : %s \n", mp->name);
    fprintf(fp, " size         : %u \n", mp->size);
    fprintf(fp, " cache        : %u \n", mp->cache_size);
    fprintf(fp, " nb_mem_chunks: %u \n", mp->nb_mem_chunks);

    fprintf(fp, " avail/in_use : %u/%u \n",
        rte_mempool_avail_count(mp), rte_mempool_in_use_count(mp));
}

inline uint32_t pool_use_count(mempool* mp)
{
    return rte_mempool_in_use_count(mp);
}

inline uint32_t pool_size(mempool* mp)
{
    return mp->size;
}


} /* namespace stcp */
