
#pragma once

#include <stcp/dpdk.h>


namespace protocol {




class proto_module {
private:
    dpdk::pkt_queue rx;
    dpdk::pkt_queue tx;

public:
    void        rx_in(struct rte_mbuf* node); // enque packet(s) to que
    void        rx_drop();                       // free a packet that was dequeued from que
    rte_mbuf*   rx_out();                     // deque a packet that wasnt freed from que
    size_t      rx_size();                       // returns size of que

    void        tx_in(struct rte_mbuf* node); // enque packet(s) to que
    void        tx_drop();                       // free a packet that was dequeued from que
    rte_mbuf*   tx_out();                     // deque a packet that wasnt freed from que
    size_t      tx_size();                       // returns size of que
};
 



} /* namespace protocol */

