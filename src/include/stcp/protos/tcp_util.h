
#pragma once

#include <assert.h>
#include <stcp/protos/tcp.h>
#include <stcp/mempool.h>
#include <stcp/config.h>
#include <stcp/arch/dpdk/device.h>
#define UNUSED(x) (void)(x)



namespace slank {




/*
 * msg: must point iphdr
 */
inline tcpip* mtod_tih(mbuf* msg)
{
    return mbuf_mtod<tcpip*>(msg);
}
inline uint16_t cksum_tih(tcpip* tih)
{
    return ipv4_udptcp_cksum(&tih->ip, &tih->tcp);
}
inline uint16_t data_len(const tcpip* tih)
{
    uint16_t iptotlen = ntoh16(tih->ip.total_length);
    uint16_t iphlen = (tih->ip.version_ihl & 0x0f)<<2;
    uint16_t tcphlen  = ((tih->tcp.data_off>>4)<<2);
    return iptotlen - iphlen - tcphlen;
}
inline uint16_t opt_len(const tcpip* tih)
{
    uint16_t len = tih->tcp.data_off>>2;
    len -= sizeof(stcp_tcp_header);
    return len;
}
inline void swap_port(tcpip* tih)
{
    uint16_t tmp   = tih->tcp.sport;
    tih->tcp.sport = tih->tcp.dport;
    tih->tcp.dport = tmp;
}
inline bool HAVE(tcpip* tih, tcpflag type)
{
    return ((tih->tcp.flags & type) != 0x00);
}
inline const char* tcpstate2str(tcpstate state)
{
    switch (state) {
        case TCPS_CLOSED:      return "CLOSED";
        case TCPS_LISTEN:      return "LISTEN";
        case TCPS_SYN_SENT:    return "SYN_SENT";
        case TCPS_SYN_RCVD:    return "SYN_RCVD";
        case TCPS_ESTABLISHED: return "ESTABLISHED";
        case TCPS_FIN_WAIT_1:  return "FIN_WAIT_1";
        case TCPS_FIN_WAIT_2:  return "FIN_WAIT_2";
        case TCPS_CLOSE_WAIT:  return "CLOSE_WAIT";
        case TCPS_CLOSING:     return "CLOSING";
        case TCPS_LAST_ACK:    return "LAST_ACK";
        case TCPS_TIME_WAIT:   return "TIME_WAIT";
        default:               return "UNKNOWN";
    }
}
inline const char* sockstate2str(socketstate state)
{
    switch (state) {
        case SOCKS_USE :    return "USE";
        case SOCKS_UNUSE:   return "UNUSE";
        case SOCKS_WAITACCEPT: return "WAITACCEPT";
        default:            return "UNKNOWN";
    }
}




} /* namespace slank */
