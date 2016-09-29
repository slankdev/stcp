

#include <stcp/rte.h>
#include <stcp/icmp.h>
#include <stcp/socket.h>
#include <stcp/stcp.h>

namespace slank {

void icmp_module::stat()
{
    m.stat();
    printf("\n");
}

void icmp_module::proc()
{
    mbuf* msg = rx_pop();
    stcp_icmp_header* ih 
        = reinterpret_cast<stcp_icmp_header*>(mbuf_push(msg, sizeof(stcp_icmp_header)));

    switch (ih->icmp_type) {
        case STCP_ICMP_ECHO:
        {
            /* reply icmp packet */
            ih->icmp_type  = STCP_ICMP_ECHOREPLY;
            ih->icmp_cksum = 0x00;
            ih->icmp_cksum = rte::raw_cksum(ih, rte::pktmbuf_data_len(msg)); 

            // TODO KOKOKARA-------------
            
            // tx_push(msg, );
            break;
        }
        case STCP_ICMP_ECHOREPLY:
        {
            /* nop */
            drop(msg);
            break;
        }
        default:
        {
            drop(msg);
            std::string errstr = "not support icmp type" + std::to_string(ih->icmp_type);
            throw slankdev::exception(errstr.c_str());
            break;
        }
    }
}

void icmp_module::tx_push(mbuf* msg, const stcp_sockaddr* dst)
{
    core::instance().ip.tx_push(msg, dst, STCP_IPPROTO_ICMP);
}

void icmp_module::rx_push(mbuf* msg)
{
    m.rx_push(msg);
}


} /* namespace slank */
