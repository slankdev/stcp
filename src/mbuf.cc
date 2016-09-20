


#include <stdint.h>
#include <stddef.h>
#include <stcp/config.h>


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


void copy_to_mbuf(mbuf* mbuf, const void* buf, size_t bufsize)
{
    // if (mbuf->pkt_len < bufsize) {
        /* 
         * TODO 
         * Support realloc mbuf
         */
        // throw slankdev::exception("mbuf do not have such space");
    // }
    mbuf->pkt_len  = bufsize;
    mbuf->data_len = bufsize;
    memcpy(rte::pktmbuf_mtod<uint8_t*>(mbuf), buf, bufsize);

}

} /* namespace */
