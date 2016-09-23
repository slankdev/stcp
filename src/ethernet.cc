

#include <stcp/rte.h>
#include <stcp/stcp.h>
#include <stcp/ethernet.h>
#include <string>

namespace slank {


uint16_t get_ether_type(mbuf* msg)
{
    stcp_ether_header* eh;
    eh = rte::pktmbuf_mtod<stcp_ether_header*>(msg);
    return rte::bswap16(eh->type);
}

void ether_module::tx_push(uint8_t port, mbuf* msg, const stcp_sockaddr* dst)
{
    
    uint8_t ether_src[6];
    uint8_t ether_dst[6];
    uint16_t ether_type;

    switch (dst->sa_fam) {
        case STCP_AF_INET:
        {
            ether_type = htons(STCP_ETHERTYPE_IP);

            if (msg->udata64 == ARPREQ_ALREADY_SENT) { // The arpreq was already sent
                arp.arp_resolv(port, dst, ether_dst, true);
            } else {
                arp.arp_resolv(port, dst, ether_dst);
            }

            uint8_t zeroaddr[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            if (memcmp(ether_dst, zeroaddr, sizeof zeroaddr) == 0) {
                msg->udata64 = ARPREQ_ALREADY_SENT;
                arp.wait.push(wait_ent(port, msg, *dst));
                return;
            }

            break;
        }
        case STCP_AF_ARP:
        {
            stcp_arphdr* ah = rte::pktmbuf_mtod<stcp_arphdr*>(msg);
            switch(rte::bswap16(ah->operation)) {
                case STCP_ARPOP_REQUEST:
                case STCP_ARPOP_REPLY:
                    ether_type = htons(STCP_ETHERTYPE_ARP);
                    break;
                case STCP_ARPOP_REVREQUEST:
                case STCP_ARPOP_REVREPLY:
                    ether_type = htons(STCP_ETHERTYPE_REVARP);
                    break;
                default:
                    std::string errstr = 
                        "not support arp operation " + std::to_string(rte::bswap16(ah->operation));
                    throw slankdev::exception(errstr.c_str());
                    break;
            }

            const uint8_t zero[6] = {0,0,0,0,0,0};
            const uint8_t bcast[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
            if (memcmp(&ah->hwdst, zero, sizeof zero) == 0 ||
                memcmp(&ah->hwdst, bcast, sizeof bcast) == 0) {
                memcpy(ether_dst, bcast, sizeof ether_dst);
            } else {
                memcpy(ether_dst, &ah->hwdst, sizeof ether_dst);
            }

            break;
        }
        default:
        {
            std::string errstr = "not support address family " + std::to_string(dst->sa_fam);
            throw slankdev::exception(errstr.c_str());
            break;
        }
    }

    
    stcp_ether_header* eh = 
        reinterpret_cast<stcp_ether_header*>(mbuf_push(msg, sizeof(stcp_ether_header)));

    core& c = core::instance();
    memset(ether_src, 0, sizeof(ether_src));
    for (ifaddr& ifa : c.dpdk.devices[port].addrs) {
        if (ifa.family == STCP_AF_LINK) {
            for (int i=0; i<6; i++)
                ether_src[i] = ifa.raw.sa_data[i];
            break;
        }
    }
    for (int i=0; i<6; i++) {
        eh->dst.addr_bytes[i] = ether_dst[i];
        eh->src.addr_bytes[i] = ether_src[i];
    }
    eh->type = ether_type;

    m.tx_push(msg);
}


void ether_module::sendto(const void* buf, size_t bufsize, const stcp_sockaddr* dst)
{
    mbuf* msg = 
        rte::pktmbuf_alloc(::slank::core::instance().dpdk.get_mempool());
    copy_to_mbuf(msg, buf, bufsize);
 
    tx_push(0, msg, dst);
}



void ether_module::proc() 
{
    while (m.rx_size() > 0) {
        mbuf* msg = rx_pop();
        uint16_t etype = get_ether_type(msg);
        mbuf_pull(msg, sizeof(stcp_ether_header));

        switch (etype) {
            case 0x0800:
            {
                ip.rx_push(msg);
                break;
            }
            case 0x0806:
            {
                arp.rx_push(msg);
                break;
            }
            default:
            {
                drop(msg);
                break;
            }
        }

    }

    while (m.tx_size() > 0) {
        core& c = core::instance();
        mbuf* msg = m.tx_pop();
        for (ifnet& dev : c.dpdk.devices) {
            dev.tx_push(msg);
        }
    }


    if (arp.wait.size() > 0) {
        wait_ent e = arp.wait.front();
        tx_push(e.port, e.msg, &e.dst);
        arp.wait.pop();
    }

}



} /* namespace slank */