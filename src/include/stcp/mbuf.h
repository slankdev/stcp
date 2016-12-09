
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stcp/config.h>
#include <stcp/arch/dpdk/rte.h>

namespace slank {


inline void mbuf_pull(mbuf* msg, size_t len)
{
    rte::pktmbuf_adj(msg, len);
}

inline void* mbuf_push(mbuf* msg, size_t len)
{
    rte::pktmbuf_prepend(msg, len);
    uint8_t* p = rte::pktmbuf_mtod<uint8_t*>(msg);
    return (void*)p;
}

inline mbuf* mbuf_alloc(mempool* mp)
{
    return rte::pktmbuf_alloc(mp);
}

inline void mbuf_free(mbuf* m)
{
    rte::pktmbuf_free(m);
}

inline mbuf* mbuf_clone(mbuf* m, mempool* mp)
{
    return rte::pktmbuf_clone(m, mp);
}

inline size_t mbuf_pkt_len(mbuf* m)
{
    return rte::pktmbuf_pkt_len(m);
}

inline size_t mbuf_data_len(mbuf* m)
{
    return rte::pktmbuf_data_len(m);
}

inline void mbuf_trim(mbuf* m, uint16_t len)
{
    rte::pktmbuf_trim(m, len);
}

inline void mbuf_dump(const mbuf* m, unsigned dump_len)
{
    rte::pktmbuf_dump(stcp_stdout.getfp(), m, dump_len);
}

inline bool mbuf_is_contiguous(const mbuf* msg)
{
    return rte::pktmbuf_is_contiguous(msg);
}

template<class T>
T mbuf_mtod(mbuf* msg)
{
    return rte::pktmbuf_mtod<T>(msg);
}

template<class T>
T mbuf_mtod_offset(mbuf* msg, size_t off)
{
    return rte::pktmbuf_mtod_offset<T>(msg, off);
}

} /* namespace */
