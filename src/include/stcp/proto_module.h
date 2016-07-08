
#pragma once

#include <stcp/dpdk.h>


namespace protocol {





enum trx {
    tx = 0,
    rx,
};

class proto_module {
private:
    dpdk::pkt_queue que[2];

public:
    void input(trx trx, struct rte_mbuf* mbuf);
    struct rte_mbuf* output(trx trx);
    size_t size(trx trx);
};
 



} /* namespace protocol */

