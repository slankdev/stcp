

#include <stcp/rte.h>
#include <stcp/stcp.h>
#include <stcp/ethernet.h>
#include <string>

namespace slank {



void ether_module::proc() 
{
    if (core::instance().arp.wait.size() > 0) {
        wait_ent e = core::instance().arp.wait.front();

        stcp_ether_addr ether_dst;
        bool ret = core::instance().arp.arp_resolv(e.msg->port, &e.dst, &ether_dst, true);

        if (ret) {
            tx_push(e.port, e.msg, &e.dst);
            core::instance().arp.wait.pop();
        }
    }
}


void ether_module::tx_push(uint8_t port, mbuf* msg, const stcp_sockaddr* dst)
{
    stcp_ether_addr ether_src;
    stcp_ether_addr ether_dst;
    uint16_t ether_type;

    switch (dst->sa_fam) {
        case STCP_AF_INET:
        {
            ether_type = rte::bswap16(STCP_ETHERTYPE_IP);

            bool  ret = core::instance().arp.arp_resolv(port, dst, &ether_dst);
            if (!ret) {
                wait_ent e(port, msg, *dst);
                core::instance().arp.wait.push(e);
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
                    ether_type = rte::bswap16(STCP_ETHERTYPE_ARP);
                    break;
                case STCP_ARPOP_REVREQUEST:
                case STCP_ARPOP_REVREPLY:
                    ether_type = rte::bswap16(STCP_ETHERTYPE_REVARP);
                    break;
                default:
                    std::string errstr = "not support arp operation ";
                    errstr += std::to_string(rte::bswap16(ah->operation));
                    throw exception(errstr.c_str());
                    break;
            }

            const uint8_t zero[6] = {0,0,0,0,0,0};
            const uint8_t bcast[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
            if (memcmp(&ah->hwdst, zero, sizeof zero) == 0 ||
                memcmp(&ah->hwdst, bcast, sizeof bcast) == 0) {
                memcpy(&ether_dst, bcast, sizeof ether_dst);
            } else {
                memcpy(&ether_dst, &ah->hwdst, sizeof ether_dst);
            }

            break;
        }
        default:
        {
            std::string errstr = "not support address family " + std::to_string(dst->sa_fam);
            throw exception(errstr.c_str());
            break;
        }
    }

    
    stcp_ether_header* eh = 
        reinterpret_cast<stcp_ether_header*>(mbuf_push(msg, sizeof(stcp_ether_header)));

    core& c = core::instance();
    memset(&ether_src, 0, sizeof(ether_src));
    for (ifaddr& ifa : c.dpdk.devices[port].addrs) {
        if (ifa.family == STCP_AF_LINK) {
            for (int i=0; i<6; i++)
                ether_src.addr_bytes[i] = ifa.raw.sa_data[i];
            break;
        }
    }
    for (int i=0; i<6; i++) {
        eh->dst.addr_bytes[i] = ether_dst.addr_bytes[i];
        eh->src.addr_bytes[i] = ether_src.addr_bytes[i];
    }
    eh->type = ether_type;

    tx_cnt++;
    for (ifnet& dev : core::instance().dpdk.devices) {
        dev.tx_push(msg);
    }
}


void ether_module::sendto(const void* buf, size_t bufsize, const stcp_sockaddr* dst)
{
    mbuf* msg = 
        rte::pktmbuf_alloc(::slank::core::instance().dpdk.get_mempool());
    copy_to_mbuf(msg, buf, bufsize);
 
    tx_push(0, msg, dst);
}


void ether_module::rx_push(mbuf* msg)
{
    rx_cnt++;
    stcp_ether_header* eh = rte::pktmbuf_mtod<stcp_ether_header*>(msg);
    uint16_t etype = rte::bswap16(eh->type);
    mbuf_pull(msg, sizeof(stcp_ether_header));

    switch (etype) {
        case STCP_ETHERTYPE_IP:
        {
            core::instance().ip.rx_push(msg);
            break;
        }
        case STCP_ETHERTYPE_ARP:
        {
            core::instance().arp.rx_push(msg);
            break;
        }
        default:
        {
            rte::pktmbuf_free(msg);
            break;
        }
    }
}

void ether_module::print_stat() const
{
    stat& s = stat::instance();
    s.write("%s", "Ether module");
    s.write("\tRX Packets %zd", rx_cnt);
    s.write("\tTX Packets %zd", tx_cnt);
}


} /* namespace slank */
