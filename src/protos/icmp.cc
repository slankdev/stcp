

#include <stcp/protos/icmp.h>
#include <stcp/socket.h>
#include <stcp/stcp.h>
#include <stcp/util.h>

namespace slank {





static uint8_t* alloc_contiguous_data_from_mbufchain(mbuf* msg)
{
        uint8_t* buf = (uint8_t*)malloc("for ICMP Several buffer", mbuf_pkt_len(msg));
        mbuf* m = msg;
        uint8_t* p = buf;
        while (m) {
            memcpy(p, mbuf_mtod<void*>(m), mbuf_data_len(m));
            p += mbuf_data_len(m);
            m = m->next;
        }
        return buf;
}



void icmp_module::send_err(icmp_type type, icmp_code code, const stcp_sockaddr_in* dst, mbuf* msg)
{
    stcp_icmp_header* ih
        = reinterpret_cast<stcp_icmp_header*>(mbuf_push(msg, sizeof(stcp_icmp_header)));

    ih->icmp_type   = type;
    ih->icmp_code   = code;
    ih->icmp_cksum  = 0x0000;
    ih->icmp_ident  = 0x0000;
    ih->icmp_seq_nb = 0x0000;


    if (mbuf_is_contiguous(msg)) {
        ih->icmp_cksum = checksum((uint16_t*)ih, mbuf_pkt_len(msg));
    } else {
        uint8_t* buf = alloc_contiguous_data_from_mbufchain(msg);
        ih->icmp_cksum = checksum((uint16_t*)buf, mbuf_pkt_len(msg));
        free(buf);
    }
    core::ip.tx_push(msg, dst, STCP_IPPROTO_ICMP);
}



void icmp_module::rx_push(mbuf* msg, const stcp_sockaddr_in* src)
{

    stcp_icmp_header* ih = mbuf_mtod<stcp_icmp_header*>(msg);

    switch (ih->icmp_type) {
        case STCP_ICMP_ECHO:
        {
            ih->icmp_type  = STCP_ICMP_ECHOREPLY;
            ih->icmp_code  = 0x00;
            ih->icmp_cksum = 0x0000;

            if (mbuf_is_contiguous(msg)) {
                ih->icmp_cksum = checksum((uint16_t*)ih, mbuf_pkt_len(msg));
            } else {
                uint8_t* buf = alloc_contiguous_data_from_mbufchain(msg);
                ih->icmp_cksum = checksum((uint16_t*)buf, mbuf_pkt_len(msg));
                free(buf);
            }

            core::ip.tx_push(msg, src, STCP_IPPROTO_ICMP);
            break;
        }
        case STCP_ICMP_ECHOREPLY:
        {
            mbuf_free(msg);
            break;
        }
        default:
        {
            mbuf_free(msg);
            // std::string errstr = "not support icmp type " + std::to_string(ih->icmp_type);
            // throw exception(errstr.c_str());
            break;
        }
    }
}

void icmp_module::print_stat() const
{
    size_t rootx = screen.POS_ICMP.x;
    size_t rooty = screen.POS_ICMP.y;
    screen.move(rooty, rootx);
    screen.printwln("ICMP module");
}

} /* namespace slank */
