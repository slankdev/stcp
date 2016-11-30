


#include <stdint.h>
#include <stddef.h>
#include <stcp/config.h>
#include <stcp/stcp.h>
#include <stcp/mbuf.h>


namespace slank {


void mbuf_pull(mbuf* msg, size_t len)
{
    rte::pktmbuf_adj(msg, len);
}

void* mbuf_push(mbuf* msg, size_t len)
{
    rte::pktmbuf_prepend(msg, len);
    uint8_t* p = rte::pktmbuf_mtod<uint8_t*>(msg);
    return (void*)p;
}

mbuf* mbuf_alloc(rte_mempool* mp)
{
    return rte::pktmbuf_alloc(mp);
}

void mbuf_free(mbuf* m)
{
    rte::pktmbuf_free(m);
}

mbuf* mbuf_clone(mbuf* m)
{
    return rte::pktmbuf_clone(m, core::get_mempool());
}

size_t mbuf_pkt_len(mbuf* m)
{
    return rte::pktmbuf_pkt_len(m);
}

size_t mbuf_data_len(mbuf* m)
{
    return rte::pktmbuf_data_len(m);
}

void mbuf_trim(mbuf* m, uint16_t len)
{
    rte::pktmbuf_trim(m, len);
}

void mbuf_dump(FILE* f,const mbuf* m, unsigned dump_len)
{
    rte::pktmbuf_dump(f, m, dump_len);
}


} /* namespace */
