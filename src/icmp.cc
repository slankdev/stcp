

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



static uint8_t* alloc_contiguous_data_from_mbufchain(mbuf* msg)
{
        uint8_t* buf = (uint8_t*)rte::malloc("for ICMP Several buffer",
                rte::pktmbuf_pkt_len(msg), 0);
        mbuf* m = msg;
        uint8_t* p = buf;
        while (m) {
            rte::memcpy(p, rte::pktmbuf_mtod<void*>(m), rte::pktmbuf_data_len(m));
            p += rte::pktmbuf_data_len(m);
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


    if (rte::pktmbuf_is_contiguous(msg)) {
        ih->icmp_cksum = checksum((uint16_t*)ih, rte::pktmbuf_pkt_len(msg));
    } else {
        uint8_t* buf = alloc_contiguous_data_from_mbufchain(msg);
        ih->icmp_cksum = checksum((uint16_t*)buf, rte::pktmbuf_pkt_len(msg));
        rte::free(buf);
    }
    core::instance().ip.tx_push(msg, dst, STCP_IPPROTO_ICMP);
}



void icmp_module::rx_push(mbuf* msg, const stcp_sockaddr_in* src)
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

            if (rte::pktmbuf_is_contiguous(msg)) {
                ih->icmp_cksum = checksum((uint16_t*)ih, rte::pktmbuf_pkt_len(msg));
            } else {
                uint8_t* buf = alloc_contiguous_data_from_mbufchain(msg);
                ih->icmp_cksum = checksum((uint16_t*)buf, rte::pktmbuf_pkt_len(msg));
                rte::free(buf);
            }

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

void icmp_module::print_stat() const
{
    stat& s = stat::instance();
    s.write("ICMP module");
    s.write("\tRX Packets %zd", rx_cnt);
    s.write("\tTX Packets %zd", tx_cnt);
}

} /* namespace slank */
