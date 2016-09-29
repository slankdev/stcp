


#pragma once

#include <stcp/dpdk.h>
#include <stcp/protocol.h>
#include <stcp/config.h>


namespace slank {


struct stcp_icmp_header {
        uint8_t  icmp_type;   /* ICMP packet type. */
        uint8_t  icmp_code;   /* ICMP packet code. */
        uint16_t icmp_cksum;  /* ICMP packet checksum. */
        uint16_t icmp_ident;  /* ICMP packet identifier. */
        uint16_t icmp_seq_nb; /* ICMP packet sequence number. */
} __attribute__((__packed__));





class icmp_module {
private:
    proto_module m;

public:
    icmp_module() {m.name = "ICMP";}
    void init() {m.init();}

    void stat();
    void proc();

    void rx_push(mbuf* msg);
    void tx_push(mbuf* msg, const stcp_sockaddr* dst);
    mbuf* rx_pop() {return m.rx_pop();}
    mbuf* tx_pop() {return m.tx_pop();}
    void drop(mbuf* msg) {m.drop(msg);}
};



enum icmp_type : uint8_t {
    STCP_ICMP_ECHOREPLY                   = 0,   /* echo reply                */
    STCP_ICMP_UNREACH                     = 3,   /* dest unreachable codes:   */
    STCP_ICMP_UNREACH_NET                 = 0,   /* bad net                   */
    STCP_ICMP_UNREACH_HOST                = 1,   /* bad host                  */
    STCP_ICMP_UNREACH_PROTOCOL            = 2,   /* bad protocol              */
    STCP_ICMP_UNREACH_PORT                = 3,   /* bad port                  */
    STCP_ICMP_UNREACH_NEEDFRAG            = 4,   /* IP_DF caused drop         */
    STCP_ICMP_UNREACH_SRCFAIL             = 5,   /* src route failed          */
    STCP_ICMP_UNREACH_NET_UNKNOWN         = 6,   /* unknown net               */
    STCP_ICMP_UNREACH_HOST_UNKNOWN        = 7,   /* unknown host              */
    STCP_ICMP_UNREACH_ISOLATED            = 8,   /* src host isolated         */
    STCP_ICMP_UNREACH_NET_PROHIB          = 9,   /* prohibited access         */
    STCP_ICMP_UNREACH_HOST_PROHIB         = 10,  /* ditto                     */
    STCP_ICMP_UNREACH_TOSNET              = 11,  /* bad tos for net           */
    STCP_ICMP_UNREACH_TOSHOST             = 12,  /* bad tos for host          */
    STCP_ICMP_UNREACH_FILTER_PROHIB       = 13,  /* admin prohib              */
    STCP_ICMP_UNREACH_HOST_PRECEDENCE     = 14,  /* host prec vio.            */
    STCP_ICMP_UNREACH_PRECEDENCE_CUTOFF   = 15,  /* prec cutoff               */
    STCP_ICMP_SOURCEQUENCH                = 4,   /* packet lost slow down     */
    STCP_ICMP_REDIRECT                    = 5,   /* shorter route codes:      */
    STCP_ICMP_REDIRECT_NET                = 0,   /* for network               */
    STCP_ICMP_REDIRECT_HOST               = 1,   /* for host                  */
    STCP_ICMP_REDIRECT_TOSNET             = 2,   /* for tos and net           */
    STCP_ICMP_REDIRECT_TOSHOST            = 3,   /* for tos and host          */
    STCP_ICMP_ALTHOSTADDR                 = 6,   /* alternate host address    */
    STCP_ICMP_ECHO                        = 8,   /* echo service              */
    STCP_ICMP_ROUTERADVERT                = 9,   /* router advertisement      */
    STCP_ICMP_ROUTERADVERT_NORMAL         = 0,   /* normal advertisement      */
    STCP_ICMP_ROUTERADVERT_NOROUTE_COMMON = 16,  /* selective routing         */
    STCP_ICMP_ROUTERSOLICIT               = 10,  /* router solicitation       */
    STCP_ICMP_TIMXCEED                    = 11,  /* time exceeded code:       */
    STCP_ICMP_TIMXCEED_INTRANS            = 0,   /* ttl=0 in transit          */
    STCP_ICMP_TIMXCEED_REASS              = 1,   /* ttl=0 in reass            */
    STCP_ICMP_PARAMPROB                   = 12,  /* ip header bad             */
    STCP_ICMP_PARAMPROB_ERRATPTR          = 0,   /* error at param ptr        */
    STCP_ICMP_PARAMPROB_OPTABSENT         = 1,   /* req. opt. absent          */
    STCP_ICMP_PARAMPROB_LENGTH            = 2,   /* bad length                */
    STCP_ICMP_TSTAMP                      = 13,  /* timestamp request         */
    STCP_ICMP_TSTAMPREPLY                 = 14,  /* timestamp reply           */
    STCP_ICMP_IREQ                        = 15,  /* information request       */
    STCP_ICMP_IREQREPLY                   = 16,  /* information reply         */
    STCP_ICMP_MASKREQ                     = 17,  /* address mask request      */
    STCP_ICMP_MASKREPLY                   = 18,  /* address mask reply        */
    STCP_ICMP_TRACEROUTE                  = 30,  /* traceroute                */
    STCP_ICMP_DATACONVERR                 = 31,  /* data conversion error     */
    STCP_ICMP_MOBILE_REDIRECT             = 32,  /* mobile host redirect      */
    STCP_ICMP_IPV6_WHEREAREYOU            = 33,  /* IPv6 where-are-you        */
    STCP_ICMP_IPV6_IAMHERE                = 34,  /* IPv6 i-am-here            */
    STCP_ICMP_MOBILE_REGREQUEST           = 35,  /* mobile registration req   */
    STCP_ICMP_MOBILE_REGREPLY             = 36,  /* mobile registration reply */
    STCP_ICMP_SKIP                        = 39,  /* SKIP                      */
    STCP_ICMP_PHOTURIS                    = 40,  /* Photuris                  */
    STCP_ICMP_PHOTURIS_UNKNOWN_INDEX      = 1,   /* unknown sec index         */
    STCP_ICMP_PHOTURIS_AUTH_FAILED        = 2,   /* auth failed               */
    STCP_ICMP_PHOTURIS_DECRYPT_FAILED     = 3,   /* decrypt failed            */
};


} /* namespace slank */
