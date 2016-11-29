
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stcp/rte.h>

namespace slank {

void  mbuf_pull(mbuf* msg, size_t len);
void* mbuf_push(mbuf* msg, size_t len);
void mbuf_free(mbuf* m);
mbuf* mbuf_clone(mbuf* m);
size_t mbuf_pkt_len(mbuf* m);
size_t mbuf_data_len(mbuf* m);
void mbuf_trim(mbuf* m, uint16_t len);

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
