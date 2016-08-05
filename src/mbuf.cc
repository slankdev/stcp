


#include <stdint.h>
#include <stddef.h>
#include <stcp/rte.h>


namespace slank {
    

void mbuf_pull(struct rte_mbuf* msg, size_t len)
{
    rte::pktmbuf_adj(msg, len);
}


void* mbuf_push(struct rte_mbuf* msg, size_t len)
{
    rte::pktmbuf_prepend(msg, len);
    uint8_t* p = rte::pktmbuf_mtod<uint8_t*>(msg);
    return (void*)p;
}



} /* namespace */