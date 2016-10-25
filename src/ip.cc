



#include <stcp/rte.h>
#include <stcp/ip.h>
#include <stcp/ethernet.h>
#include <stcp/socket.h>
#include <stcp/stcp.h>
#include <stcp/icmp.h>

namespace slank {



void ip_module::init()
{
    uint32_t max_flow_num   = 0x1000; // TODO hardcode
    uint32_t bucket_num     = max_flow_num;
    uint32_t bucket_entries = 16; // TODO hardcode
    uint32_t max_entries    = max_flow_num;
    uint64_t max_cycles     = (rte_get_tsc_hz() + MS_PER_S - 1) / MS_PER_S * MS_PER_S;

    frag_tbl = rte_ip_frag_table_create(
            bucket_num,
            bucket_entries,
            max_entries,
            max_cycles,
            rte_socket_id()
            );
    if (!frag_tbl) {
        throw exception("rte_ip_frag_table_create");
    }

    rte::srand(time(NULL));
}


void ip_module::set_ipaddr(const stcp_in_addr* addr)
{
    myip.addr_bytes[0] = addr->addr_bytes[0];
    myip.addr_bytes[1] = addr->addr_bytes[1];
    myip.addr_bytes[2] = addr->addr_bytes[2];
    myip.addr_bytes[3] = addr->addr_bytes[3];
}


void ip_module::rx_push(mbuf* msg)
{
    rx_cnt++;
    stcp_ip_header* ih
        = rte::pktmbuf_mtod<stcp_ip_header*>(msg);

    size_t trailer_len = rte::pktmbuf_data_len(msg) - rte::bswap16(ih->total_length);
    rte::pktmbuf_trim(msg, trailer_len);


    /*
     * TODO hardcode
     */
    stcp_in_addr bcast;
    bcast.addr_bytes[0] = 0xff;
    bcast.addr_bytes[1] = 0xff;
    bcast.addr_bytes[2] = 0xff;
    bcast.addr_bytes[3] = 0xff;

    if (myip != ih->dst && bcast != ih->dst) {
        not_to_me++;
        rte::pktmbuf_free(msg);
        return;
    }

    if (rte::ipv4_frag_pkt_is_fragmented(ih)) {
        mbuf_push(msg, sizeof(stcp_ether_header));

        msg->l2_len = sizeof(stcp_ether_header);
        msg->l3_len = sizeof(stcp_ip_header);
        mbuf* reasmd_msg = rte::ipv4_frag_reassemble_packet(
                frag_tbl, &dr, msg, rte_rdtsc(),
                reinterpret_cast<struct ipv4_hdr*>(ih));

        if (reasmd_msg == NULL) {
            return;
        }
        msg = reasmd_msg;

        mbuf_pull(msg, sizeof(stcp_ether_header));
        ih = rte::pktmbuf_mtod<stcp_ip_header*>(msg);
    }

    mbuf_pull(msg, sizeof(stcp_ip_header));

    stcp_sockaddr_in src;
    src.sin_addr = ih->src;
    uint8_t protocol = ih->next_proto_id;
    switch (protocol) {
        case STCP_IPPROTO_ICMP:
        {
            core::icmp.rx_push(msg, &src);
            break;
        }
#if 1
        case STCP_IPPROTO_TCP:
        {
            core::tcp.rx_push(msg, &src);
            break;
        }
#endif
        case STCP_IPPROTO_UDP:
        {
            core::udp.rx_push(msg, &src);
            break;
        }
        default:
        {
            mbuf_push(msg, sizeof(stcp_ip_header));
            core::icmp.send_err(STCP_ICMP_UNREACH,
                    STCP_ICMP_UNREACH_PROTOCOL, &src, msg);
            break;
        }
    }
}


void ip_module::ioctl(uint64_t request, void* args)
{
    switch (request) {
        case STCP_SIOCADDRT:
        {
            const stcp_rtentry* rt = reinterpret_cast<const stcp_rtentry*>(args);
            ioctl_siocaddrt(rt);
            break;
        }
        case STCP_SIOCADDGW:
        {
            stcp_rtentry* rt = reinterpret_cast<stcp_rtentry*>(args);
            ioctl_siocaddgw(rt);
            break;
        }
        case STCP_SIOCDELRT:
        {
            stcp_rtentry* rt = reinterpret_cast<stcp_rtentry*>(args);
            ioctl_siocdelrt(rt);
            break;
        }
        case STCP_SIOCGETRTS:
        {
            std::vector<stcp_rtentry>** table =
                reinterpret_cast<std::vector<stcp_rtentry>**>(args);
            ioctl_siocgetrts(table);
            break;
        }
        default:
        {
            throw exception("invalid arguments");
            break;
        }
    }
}





/*
 * This function evaluate all elements
 * of stcp_rtentry.
 */
void ip_module::ioctl_siocaddrt(const stcp_rtentry* rt)
{
    rttable.push_back(*rt);
}


/*
 * This function evaluate only these.
 *  - rt.rt_port
 *  - rt.rt_gateway
 */
void ip_module::ioctl_siocaddgw(stcp_rtentry* rt)
{
    rt->rt_route.inet_addr(0, 0, 0, 0);
    rt->rt_genmask.inet_addr(0, 0, 0, 0);
    rt->rt_flags = STCP_RTF_GATEWAY;
    rttable.push_back(*rt);
}


void ip_module::ioctl_siocdelrt(const stcp_rtentry* rt)
{
    for (size_t i=0; i<rttable.size(); i++) {
        if (*rt == rttable[i]) {
            rttable.erase(rttable.begin() + i);
            return;
        }
    }
    throw exception("not found routing info");
}

void ip_module::ioctl_siocgetrts(std::vector<stcp_rtentry>** table)
{
    *table = &rttable;
}

void ip_module::route_resolv(const stcp_sockaddr_in* dst, stcp_sockaddr_in* next, uint8_t* port)
{
    dpdk_core& dpdk = core::dpdk;

    for (size_t i=0; i<dpdk.devices.size(); i++) {
        if (is_linklocal(i, dst)) {
            *next = *dst;
            *port = i;
            return;
        }
    }

    for (stcp_rtentry& rt : rttable) {
        if (rt.rt_flags & STCP_RTF_GATEWAY) {
            *next = rt.rt_gateway;
            *port = rt.rt_port;
            return;
        }
    }

    throw exception("not found route");
}

bool ip_module::is_linklocal(uint8_t port, const stcp_sockaddr_in* addr)
{
    dpdk_core& dpdk = core::dpdk;
    stcp_sockaddr inaddr(STCP_AF_INET);
    stcp_sockaddr inmask(STCP_AF_INET);
    stcp_sockaddr innet(STCP_AF_INET);
    bool inaddr_exist = false;
    bool inmask_exist = false;

    for (ifaddr& ifa : dpdk.devices[port].addrs) {
        if (ifa.family == STCP_AF_INET) {
            inaddr = ifa.raw;
            inaddr_exist = true;
        }
        if (ifa.family == STCP_AF_INMASK) {
            inmask = ifa.raw;
            inmask_exist = true;
        }
    }

    if (!inaddr_exist || !inmask_exist) {
        throw exception("inaddr or inmask is not exist");
    }

    stcp_sockaddr_in* inaddr_sin = reinterpret_cast<stcp_sockaddr_in*>(&inaddr);
    stcp_sockaddr_in* inmask_sin = reinterpret_cast<stcp_sockaddr_in*>(&inmask);
    stcp_sockaddr_in* innet_sin  = reinterpret_cast<stcp_sockaddr_in*>(&innet );

    for (int i=0; i<4; i++) {
        innet_sin->sin_addr.addr_bytes[i] =   inaddr_sin->sin_addr.addr_bytes[i]
                                            & inmask_sin->sin_addr.addr_bytes[i];
    }

    for (int i=0; i<4; i++) {
        if ((inmask_sin->sin_addr.addr_bytes[i] & addr->sin_addr.addr_bytes[i])
                != innet_sin->sin_addr.addr_bytes[i])
            return false;
    }
    return true;
}


void ip_module::tx_push(mbuf* msg, const stcp_sockaddr_in* dst, ip_l4_protos proto)
{
    tx_cnt++;

    stcp_ip_header* ih
        = reinterpret_cast<stcp_ip_header*>(mbuf_push(msg, sizeof(stcp_ip_header)));

    ih->version_ihl       = 0x45;
    ih->type_of_service   = 0x00;
    ih->total_length      = rte::bswap16(rte::pktmbuf_pkt_len(msg));
    ih->packet_id         = rte::bswap16(rte::rand() % 0xffff);

    if (rte::pktmbuf_pkt_len(msg) > core::dpdk.ipv4_mtu_default) {
        ih->fragment_offset = rte::bswap16(0x0000);
    } else {
        ih->fragment_offset   = rte::bswap16(0x4000);
    }

    ih->time_to_live      = ip_module::ttl_default;
    ih->next_proto_id     = proto;
    ih->hdr_checksum      = 0x00;

    /*
     * TODO Not smart implementation
     */
    ih->src.addr_bytes[0] = myip.addr_bytes[0];
    ih->src.addr_bytes[1] = myip.addr_bytes[1];
    ih->src.addr_bytes[2] = myip.addr_bytes[2];
    ih->src.addr_bytes[3] = myip.addr_bytes[3];
    ih->dst.addr_bytes[0] = dst->sin_addr.addr_bytes[0];
    ih->dst.addr_bytes[1] = dst->sin_addr.addr_bytes[1];
    ih->dst.addr_bytes[2] = dst->sin_addr.addr_bytes[2];
    ih->dst.addr_bytes[3] = dst->sin_addr.addr_bytes[3];

    ih->hdr_checksum = rte_ipv4_cksum((const struct ipv4_hdr*)ih);

    mbuf* msgs[100]; // TODO hardcode about mbuf[] size
    memset(msgs, 0, sizeof msgs);
    uint32_t nb = rte::ipv4_fragment_packet(msg, &msgs[0], 10,
            core::dpdk.ipv4_mtu_default,
            core::dpdk.get_mempool(),
            core::dpdk.get_mempool());

    stcp_sockaddr_in next;
    uint8_t port;
    route_resolv(dst, &next, &port);
    next.sin_fam = STCP_AF_INET;

    if (nb > 1) { /* packet was fragmented */
        rte::pktmbuf_free(msg);

        for (size_t i=0; i<nb; i++) {
            stcp_ip_header* iph = rte::pktmbuf_mtod<stcp_ip_header*>(msgs[i]);
            iph->hdr_checksum = rte_ipv4_cksum(reinterpret_cast<const struct ipv4_hdr*>(iph));

            msgs[i]->port = port;
            core::ether.tx_push(msgs[i]->port, msgs[i],
                    reinterpret_cast<stcp_sockaddr*>(&next));
        }
    } else { /* packet was not fragmented */
        msg->port = port;
        core::ether.tx_push(msg->port, msg,
                reinterpret_cast<stcp_sockaddr*>(&next));
    }
}


void ip_module::print_stat() const
{
    stat& s = stat::instance();
    s.write("IP module");
    s.write("\tRX Packets %zd", rx_cnt);
    s.write("\tTX Packets %zd", tx_cnt);
    s.write("\tDrops      %zd", not_to_me);
    s.write("");
    s.write("\tRouting-Table");
    s.write("\t%-16s%-16s%-16s%-6s%-3s", "Destination", "Gateway", "Genmask", "Flags", "if");
    for (const stcp_rtentry& rt : rttable) {
        std::string str_dest;
        if (rt.rt_flags & STCP_RTF_GATEWAY) {
            str_dest = "defalt";
        } else if (rt.rt_flags & STCP_RTF_LOCAL) {
            str_dest = "link-local";
        } else {
            str_dest = rt.rt_route.c_str();
        }

        std::string flag_str = "";
        if (rt.rt_flags & STCP_RTF_GATEWAY  )   flag_str += "G";
        if (rt.rt_flags & STCP_RTF_MASK     )   flag_str += "M";
        if (rt.rt_flags & STCP_RTF_LOCAL    )   flag_str += "L";
        if (rt.rt_flags & STCP_RTF_BROADCAST)   flag_str += "B";

        std::string gateway_str;
        if (rt.rt_flags & STCP_RTF_LOCAL) {
            gateway_str = "*";
        } else {
            gateway_str = rt.rt_gateway.c_str();
        }
        s.write("\t%-16s%-16s%-16s%-6s%-3u",
                str_dest.c_str(),
                gateway_str.c_str(),
                rt.rt_genmask.c_str(),
                flag_str.c_str(),
                rt.rt_port);
    }
}

};
