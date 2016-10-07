

#include <stcp/rte.h>
#include <stcp/icmp.h>
#include <stcp/socket.h>
#include <stcp/stcp.h>

namespace slank {



/* 
 * TODO 
 * XXX review this function
 * I copyed from Geek NA Page.
 * http://www.geekpage.jp
 */
static uint16_t checksum(uint16_t* buf, size_t bufsz)
{
    uint32_t sum = 0;

    while (bufsz > 1) {
        sum += *buf;
        buf++;
        bufsz -= 2;
    }

    if (bufsz == 1) {
        sum += *(uint16_t *)buf;
    }

    sum = (sum & 0xffff) + (sum >> 16);
    sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}


void icmp_module::rx_push(mbuf* msg, const stcp_sockaddr* src)
{
    rx_cnt++;

    stcp_icmp_header* ih 
        = rte::pktmbuf_mtod<stcp_icmp_header*>(msg);

    switch (ih->icmp_type) {
        case STCP_ICMP_ECHO:
        {
            ih->icmp_type  = STCP_ICMP_ECHOREPLY;
            ih->icmp_code  = 0x00;
            ih->icmp_cksum = 0x0000;
            ih->icmp_cksum = checksum((uint16_t*)ih, rte::pktmbuf_data_len(msg));

            core::instance().ip.tx_push(msg, src, STCP_IPPROTO_ICMP);
            tx_cnt++;
            break;
        }
        case STCP_ICMP_ECHOREPLY:
        {
            rte::pktmbuf_free(msg);
            break;
        }
        default:
        {
            rte::pktmbuf_free(msg);
            // std::string errstr = "not support icmp type " + std::to_string(ih->icmp_type);
            // throw exception(errstr.c_str());
            break;
        }
    }
}


} /* namespace slank */
