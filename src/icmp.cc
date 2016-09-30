

#include <stcp/rte.h>
#include <stcp/icmp.h>
#include <stcp/socket.h>
#include <stcp/stcp.h>

namespace slank {

void icmp_module::stat()
{
    printf("\n");
    printf("ICMP module\n");
    printf("\tRX Packets %zd\n", rx_cnt);
    printf("\tTX Packets %zd\n", tx_cnt);
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

#if 1 /* XXX DPDK's function that calc checksum can not work correctry. */
            ih->icmp_cksum = slankdev::checksum(ih, rte::pktmbuf_data_len(msg));
            ih->icmp_cksum = slankdev::checksum(ih, rte::pktmbuf_data_len(msg));
#else
            ih->icmp_cksum = rte::raw_cksum(ih, rte::pktmbuf_data_len(msg));
            ih->icmp_cksum = rte::raw_cksum(ih, rte::pktmbuf_data_len(msg));
#endif

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
            std::string errstr = "not support icmp type " + std::to_string(ih->icmp_type);
            throw exception(errstr.c_str());
            break;
        }
    }
}


} /* namespace slank */
