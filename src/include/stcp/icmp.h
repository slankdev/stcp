


#pragma once

#include <stcp/dpdk.h>
#include <stcp/protocol.h>
#include <stcp/config.h>


namespace slank {


struct stcp_icmp_header {
        uint8_t  icmp_type;   /* ICMP packet type.            */
        uint8_t  icmp_code;   /* ICMP packet code.            */
        uint16_t icmp_cksum;  /* ICMP packet checksum.        */
        uint16_t icmp_ident;  /* ICMP packet identifier.      */
        uint16_t icmp_seq_nb; /* ICMP packet sequence number. */
} __attribute__((__packed__));




enum icmp_code : uint8_t {
    /* unreachable  */
    STCP_ICMP_UNREACH_PROTOCOL            = 2,   /* bad protocol              */
    STCP_ICMP_UNREACH_PORT                = 3,   /* bad port                  */

    // #<{(| redirect     |)}>#
    // STCP_ICMP_REDIRECT_NET                = 0,   #<{(| for network               |)}>#
    // STCP_ICMP_REDIRECT_HOST               = 1,   #<{(| for host                  |)}>#
    // STCP_ICMP_REDIRECT_TOSNET             = 2,   #<{(| for tos and net           |)}>#
    // STCP_ICMP_REDIRECT_TOSHOST            = 3,   #<{(| for tos and host          |)}>#

    // #<{(| timeexceeded |)}>#
    // STCP_ICMP_TIMXCEED_INTRANS            = 0,   #<{(| ttl=0 in transit          |)}>#
    // STCP_ICMP_TIMXCEED_REASS              = 1,   #<{(| ttl=0 in reass            |)}>#
};



enum icmp_type : uint8_t {
    STCP_ICMP_ECHOREPLY                   = 0,   /* echo reply                */
    STCP_ICMP_UNREACH                     = 3,   /* dest unreachable codes:   */
    // STCP_ICMP_REDIRECT                    = 5,   #<{(| shorter route codes:      |)}>#
    STCP_ICMP_ECHO                        = 8,   /* echo service              */
    // STCP_ICMP_TIMXCEED                    = 11,  #<{(| time exceeded code:       |)}>#
};


class icmp_module {
    friend class core;
private:
    size_t rx_cnt;
    size_t tx_cnt;

public:
    icmp_module() : rx_cnt(0), tx_cnt(0) {}

    void rx_push(mbuf* msg, const stcp_sockaddr* src);
    void send_err(icmp_type type, icmp_code code, const stcp_sockaddr* dst, mbuf* msg);
};




} /* namespace slank */
