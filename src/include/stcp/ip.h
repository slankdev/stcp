
#pragma once

#include <stcp/dpdk.h>
#include <stcp/protocol.h>



namespace slank {
    

struct stcp_ip_header {
	uint8_t  version_ihl;		/**< version and header length */
	uint8_t  type_of_service;	/**< type of service */
	uint16_t total_length;		/**< length of packet */
	uint16_t packet_id;		/**< packet ID */
	uint16_t fragment_offset;	/**< fragmentation offset */
	uint8_t  time_to_live;		/**< time to live */
	uint8_t  next_proto_id;		/**< protocol ID */
	uint16_t hdr_checksum;		/**< header checksum */
    stcp_in_addr src;
    stcp_in_addr dst;
} ;



class ip_module {
private:
    proto_module m;

public:

    ip_module() { m.name = "IP"; }
    void init() {m.init();}
    void rx_push(struct rte_mbuf* msg){m.rx_push(msg);}
    void tx_push(struct rte_mbuf* msg){m.tx_push(msg);}
    struct rte_mbuf* rx_pop() {return m.rx_pop();}
    struct rte_mbuf* tx_pop() {return m.tx_pop();}
    void drop(struct rte_mbuf* msg) {m.drop(msg);}
    void proc() {m.proc();}
    void stat() {m.stat();}
};


} /* namespace */
