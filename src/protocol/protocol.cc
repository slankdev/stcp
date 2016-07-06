


#include "protocol.h"
#include <stcp/rte.h>


namespace protocol {
   


void proto_module::rx_in(struct rte_mbuf* node)
{
    rx.enq(node);
}


void proto_module::rx_drop()
{
    rte::pktmbuf_free(rx.deq());
}


struct rte_mbuf* proto_module::rx_out()
{
    return rx.deq();
}


size_t proto_module::rx_size()
{
    return rx.size();
}

void proto_module::tx_in(struct rte_mbuf* node)
{
    tx.enq(node);
}


void proto_module::tx_drop()
{
    rte::pktmbuf_free(tx.deq());
}


struct rte_mbuf* proto_module::tx_out()
{
    return tx.deq();
}


size_t proto_module::tx_size()
{
    return tx.size();
}



} /* namespace protocol */
