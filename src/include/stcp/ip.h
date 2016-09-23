
#pragma once

#include <stcp/dpdk.h>
#include <stcp/protocol.h>
#include <stcp/config.h>



namespace slank {
 


enum : uint64_t {
    // STCP_SIOCADDRT,
    // STCP_SIOCDELRT,
    // STCP_SIOCGETRTS,
    STCP_SIOCSETGW,
};


enum : uint32_t {
    STCP_RTF_GATEWAY   = 1 << 1,
    STCP_RTF_MASK      = 1 << 2,
    STCP_RTF_LOCAL     = 1 << 4,
    STCP_RTF_BROADCAST = 1 << 5,
};



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



struct stcp_rtentry {
    stcp_sockaddr  rt_route;   /* route destination        */
    stcp_sockaddr  rt_mask;    /* netmask                  */
    stcp_sockaddr  rt_gateway; /* next hop address         */
    stcp_sockaddr  rt_ifa;     /* interface address to use */
    uint8_t        rt_port;       /* interface index to use   */
    uint32_t       rt_flags;   /* up/down?, host/net       */
    uint64_t       rt_mtu;     /* MTU for this path        */
    uint64_t       rt_rmx;     /* Num of Metrix            */

    stcp_rtentry() :
        rt_port(0), rt_flags(0), rt_mtu(0), rt_rmx(0) {}
    const char* c_str();
};





class ip_module {
private:
    proto_module m;

public:
    std::vector<stcp_rtentry> rttable;

    ip_module() { m.name = "IP"; }
    void init() {m.init();}
    void rx_push(mbuf* msg){m.rx_push(msg);}
    void tx_push(mbuf* msg){m.tx_push(msg);}
    mbuf* rx_pop() {return m.rx_pop();}
    mbuf* tx_pop() {return m.tx_pop();}
    void drop(mbuf* msg) {m.drop(msg);}
    void proc() {m.proc();}
    void stat();


public:
    void ioctl(uint64_t request, void* args);
    void ioctl_siocsetgw(const stcp_rtentry* rt);
};


} /* namespace */
