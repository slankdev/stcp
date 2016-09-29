
#pragma once

#include <stcp/dpdk.h>
#include <stcp/protocol.h>
#include <stcp/config.h>



namespace slank {
 


enum : uint64_t {
    STCP_SIOCADDRT,
    STCP_SIOCDELRT,
    STCP_SIOCGETRTS,
    STCP_SIOCADDGW,
};


enum : uint32_t {
    STCP_RTF_GATEWAY   = 1 << 0,
    STCP_RTF_MASK      = 1 << 1,
    STCP_RTF_LOCAL     = 1 << 2,
    STCP_RTF_BROADCAST = 1 << 3,
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
    stcp_sockaddr  rt_genmask;    /* netmask                  */
    stcp_sockaddr  rt_gateway; /* next hop address         */
    // stcp_sockaddr  rt_ifa;     #<{(| interface address to use |)}>#
    uint8_t        rt_port;       /* interface index to use   */
    uint32_t       rt_flags;   /* up/down?, host/net       */

    stcp_rtentry() :
        rt_port(0), rt_flags(0) {}

    bool operator==(const stcp_rtentry& rhs) const
    {
        if (rt_route   != rhs.rt_route  ) return false;
        if (rt_genmask != rhs.rt_genmask) return false;
        if (rt_gateway != rhs.rt_gateway) return false;
        if (rt_port    != rhs.rt_port   ) return false;
        if (rt_flags   != rhs.rt_flags  ) return false;
        return true;
    }
    bool operator!=(const stcp_rtentry& rhs) const
    {
        return !(*this==rhs);
    }
};





class ip_module {
private:
    proto_module m;

public:
    std::vector<stcp_rtentry> rttable;

    ip_module()
    { 
        m.name = "IP"; 
    }
    void init() {m.init();}
    void rx_push(mbuf* msg){m.rx_push(msg);}
    void tx_push(mbuf* msg, const stcp_sockaddr* dst);
    mbuf* rx_pop() {return m.rx_pop();}
    mbuf* tx_pop() {return m.tx_pop();}
    void drop(mbuf* msg) {m.drop(msg);}
    void proc();
    void stat();
    size_t tx_size() {return m.tx_size();}

    void sendto(const void* buf, size_t bufsize, const stcp_sockaddr* dst);
    void ioctl(uint64_t request, void* args);
    void route_resolv(const stcp_sockaddr* dst, stcp_sockaddr* next, uint8_t* port);

private:
    void ioctl_siocaddrt(const stcp_rtentry* rt);
    void ioctl_siocaddgw(stcp_rtentry* rt);
    void ioctl_siocdelrt(const stcp_rtentry* rt);
    void ioctl_siocgetrts(std::vector<stcp_rtentry>** table);

private:
    bool is_linklocal(uint8_t port, const stcp_sockaddr* addr);
};


} /* namespace */
