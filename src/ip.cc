



#include <stcp/rte.h>
#include <stcp/ip.h>
#include <stcp/socket.h>
#include <stcp/stcp.h>

namespace slank {


    

void ip_module::rx_push(mbuf* msg)
{
    rx_cnt++;

    stcp_ip_header* ih 
        = rte::pktmbuf_mtod<stcp_ip_header*>(msg);
    mbuf_pull(msg, sizeof(stcp_ip_header));


    // TODO hardcode
    stcp_in_addr myip;
    myip.addr_bytes[0] = 192;
    myip.addr_bytes[1] = 168;
    myip.addr_bytes[2] = 222;
    myip.addr_bytes[3] = 10;

    stcp_in_addr bcast;
    bcast.addr_bytes[0] = 0xff;
    bcast.addr_bytes[1] = 0xff;
    bcast.addr_bytes[2] = 0xff;
    bcast.addr_bytes[3] = 0xff;

    if (myip != ih->dst && bcast != ih->dst) {
        rte::pktmbuf_free(msg);
        return;
    }

    stcp_sockaddr src;
    stcp_sockaddr_in* src_sin = reinterpret_cast<stcp_sockaddr_in*>(&src);
    src_sin->sin_addr = ih->src;
    uint8_t protocol = ih->next_proto_id;
    switch (protocol) {
        case STCP_IPPROTO_ICMP:
            {
                core::instance().icmp.rx_push(msg, &src);
                break;
            }
        case STCP_IPPROTO_TCP:
            {
                rte::pktmbuf_free(msg);
                break;
            }
        case STCP_IPPROTO_UDP:
            {
                rte::pktmbuf_free(msg);
                break;
            }
        default:
            {
                rte::pktmbuf_free(msg);
                std::string errstr = "unknown l4 proto " + std::to_string(protocol);
                throw exception(errstr.c_str());
                break;
            }

    }
}

void ip_module::stat()
{
    printf("IP module\n");
    printf("\tRX Packets %zd\n", rx_cnt);
    printf("\tTX Packets %zd\n", tx_cnt);
    printf("\n");
    printf("\tRouting-Table\n");
    printf("\t%-16s%-16s%-16s%-6s%-3s\n", "Destination", "Gateway", "Genmask", "Flags", "if");
    for (stcp_rtentry& rt : rttable) {

        std::string str_dest;
        if (rt.rt_flags & STCP_RTF_GATEWAY) {
            str_dest = "defalt";
        } else if (rt.rt_flags & STCP_RTF_LOCAL) {
            str_dest = "link-local";
        } else {
            str_dest = p_sockaddr_to_str(&rt.rt_route);
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
            gateway_str = p_sockaddr_to_str(&rt.rt_gateway);
        }

        printf("\t%-16s%-16s%-16s%-6s%-3u\n",
                str_dest.c_str(),
                gateway_str.c_str(),
                p_sockaddr_to_str(&rt.rt_genmask),
                flag_str.c_str(),
                rt.rt_port);
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




void ip_module::route_resolv(const stcp_sockaddr* dst, stcp_sockaddr* next, uint8_t* port)
{
    dpdk_core& dpdk = core::instance().dpdk;

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

bool ip_module::is_linklocal(uint8_t port, const stcp_sockaddr* addr)
{
    dpdk_core& dpdk = core::instance().dpdk;
    stcp_sockaddr inaddr;
    stcp_sockaddr inmask;
    stcp_sockaddr innet;
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

    const stcp_sockaddr_in* sin = reinterpret_cast<const stcp_sockaddr_in*>(addr);
    for (int i=0; i<4; i++) {
        if ((inmask_sin->sin_addr.addr_bytes[i] & sin->sin_addr.addr_bytes[i])
                != innet_sin->sin_addr.addr_bytes[i])
            return false;
    }
    return true;
}

void ip_module::sendto(const void* buf, size_t bufsize, const stcp_sockaddr* dst, ip_l4_protos p)
{
    mbuf* msg = 
        rte::pktmbuf_alloc(::slank::core::instance().dpdk.get_mempool());
    copy_to_mbuf(msg, buf, bufsize);
 
    tx_push(msg, dst, p);
}


void ip_module::tx_push(mbuf* msg, const stcp_sockaddr* dst, ip_l4_protos proto)
{
    tx_cnt++;

    const stcp_sockaddr_in* sin = reinterpret_cast<const stcp_sockaddr_in*>(dst);
    
    stcp_ip_header* ih 
        = reinterpret_cast<stcp_ip_header*>(mbuf_push(msg, sizeof(stcp_ip_header)));
    
    ih->version_ihl       = 0x45;
    ih->type_of_service   = 0x00;
    ih->total_length      = rte::bswap16(rte::pktmbuf_data_len(msg));
    ih->packet_id         = rte::bswap16(0x7e4d);
    ih->fragment_offset   = rte::bswap16(0x4000);
    ih->time_to_live      = ip_module::ttl_default;
    ih->next_proto_id     = proto;
    ih->hdr_checksum      = 0x00; 
    ih->src.addr_bytes[0] = 192; // TODO hardcode
    ih->src.addr_bytes[1] = 168; // TODO hardcode
    ih->src.addr_bytes[2] = 222; // TODO hardcode
    ih->src.addr_bytes[3] = 10 ; // TODO hardcode
    ih->dst.addr_bytes[0] = sin->sin_addr.addr_bytes[0];
    ih->dst.addr_bytes[1] = sin->sin_addr.addr_bytes[1];
    ih->dst.addr_bytes[2] = sin->sin_addr.addr_bytes[2];
    ih->dst.addr_bytes[3] = sin->sin_addr.addr_bytes[3];

    ih->hdr_checksum = rte_ipv4_cksum((const struct ipv4_hdr*)ih);


    stcp_sockaddr next;
    uint8_t port;
    route_resolv(dst, &next, &port);
    next.sa_fam = STCP_AF_INET;

    msg->port = port;
    core::instance().ether.tx_push(msg->port, msg, &next);
}


};
