


#include <stcp/proto_module.h>
#include <stcp/rte.h>


   

void proto_module::input(trx trx, struct rte_mbuf* mbuf)
{
    que[trx].enq(mbuf);
}


struct rte_mbuf* proto_module::output(trx trx)
{
    return que[trx].deq();
}


size_t proto_module::size(trx trx)
{
    return que[trx].size();
    
}




