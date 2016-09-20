

#include <arpa/inet.h>

#include <stcp/arp.h>
#include <stcp/config.h>
#include <stcp/dpdk.h>
#include <stcp/rte.h>

#include <pgen2.h>


namespace slank {
    


static bool hw_sockaddr_is_same(const stcp_sockaddr* a, const stcp_sockaddr* b)
{
    for (int i=0; i<6; i++) {
        if (a->sa_data[i] != b->sa_data[i])
            return false;
    }
    return true;
}

static bool p_sockaddr_is_same(const stcp_sockaddr* a, const stcp_sockaddr* b)
{
    const stcp_sockaddr_in* sina = reinterpret_cast<const stcp_sockaddr_in*>(a);
    const stcp_sockaddr_in* sinb = reinterpret_cast<const stcp_sockaddr_in*>(b);
    return ( sina->sin_addr == sinb->sin_addr );
}


void arp_module::stat() 
{
    m.stat();
    printf("\n");
    printf("\tARP-chace\n");
    printf("\t%-16s %-20s %s\n", "Address", "HWaddress", "Iface");
    for (stcp_arpreq& a : table) {
        printf("\t%-16s %-20s %d\n",
                p_sockaddr_to_str(&a.arp_pa), 
                hw_sockaddr_to_str(&a.arp_ha), a.arp_ifindex);
    }
}

void arp_module::proc() 
{
    while (m.rx_size() > 0) {
        struct rte_mbuf* msg = m.rx_pop();
        struct stcp_arphdr* ah  = rte::pktmbuf_mtod<struct stcp_arphdr*>(msg);
        uint8_t port = msg->port;

        if (ah->operation == htons(2)) { // TODO hard code
            stcp_sockaddr     sa_pa;
            stcp_sockaddr     sa_ha;
            stcp_sockaddr_in *sin_pa = reinterpret_cast<stcp_sockaddr_in*>(&sa_pa);

            sin_pa->sin_addr = ah->psrc;
            for (int i=0; i<6; i++)
                sa_ha.sa_data[i] = ah->hwsrc.addr_bytes[i];

            stcp_arpreq req(&sa_pa, &sa_ha, port);
            ioctl_siocaarpent(&req);

        } else if (ah->operation == htons(1)) {
#if 0
            proc_arpreply(ah, port);
#endif
        }
        rte::pktmbuf_free(msg);
    }

    while (m.tx_size() > 0) throw slankdev::exception("Not Impl yet");
}


#if 0
static struct rte_mbuf* alloc_reply_packet(struct stcp_arphdr* ah, uint8_t port)
{
	struct ether_addr mymac;
	memset(&mymac, 0, sizeof(mymac));

	bool macfound=false;
	ifnet& dev = dpdk::instance().devices[port];
	for (ifaddr& ifa : dev.addrs) {
		if (ifa.family == STCP_AF_LINK) {
			mymac = ifa.raw.link;
			macfound = true;
		}
	}
	if (!macfound)
		throw slankdev::exception("address not found");


    dpdk& d = dpdk::instance();
	struct rte_mbuf* msg = rte::pktmbuf_alloc(d.get_mempool());
    msg->data_len = 64;
    msg->pkt_len  = 64;
	uint8_t* data = rte::pktmbuf_mtod<uint8_t*>(msg);
	struct stcp_ether_header* eh = reinterpret_cast<struct stcp_ether_header*>(data);
	eh->src = mymac;
	eh->dst = ah->hwsrc;
	eh->type = htons(0x0806);

	struct stcp_arphdr* rep_ah = 
        reinterpret_cast<struct stcp_arphdr*>(data + sizeof(struct stcp_ether_header));
	rep_ah->hwtype = htons(1);
	rep_ah->ptype  = htons(0x0800);
	rep_ah->hwlen  = 6;
	rep_ah->plen   = 4;
	rep_ah->operation = htons(2); // 2 is reply
	rep_ah->hwsrc = eh->src;
	rep_ah->psrc  = ah->pdst;
	rep_ah->hwdst = eh->dst;
	rep_ah->pdst  = ah->psrc;

	return msg;
}
#endif

#if 0
static bool is_request_to_me(struct stcp_arphdr* ah, uint8_t port)
{
	ifnet& dev = dpdk::instance().devices[port];
	for (ifaddr& ifa : dev.addrs) {
        stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&ifa.raw);
		if (ifa.family == STCP_AF_INET && sin->sin_addr==ah->pdst)
			return true;
	}
	return false;
}
#endif

#if 0
void arp_module::proc_arpreply(struct stcp_arphdr* ah, uint8_t port)
{
	if (is_request_to_me(ah, port)) {
		struct rte_mbuf* msg = alloc_reply_packet(ah, port);

		dpdk& d = dpdk::instance();
		d.devices[port].tx_push(msg);
	}
}
#endif

void arp_module::ioctl(uint64_t request, void* arg)
{
    switch (request) {
        case STCP_SIOCAARPENT:
        {
            stcp_arpreq* req = reinterpret_cast<stcp_arpreq*>(arg);
            ioctl_siocaarpent(req);
            break;
        }
        case STCP_SIOCDARPENT:
        {
            stcp_arpreq* req = reinterpret_cast<stcp_arpreq*>(arg);
            ioctl_siocdarpent(req);
            break;
        }
        case STCP_SIOCGARPENT:
        {
            std::vector<stcp_arpreq>** tbl = 
                reinterpret_cast<std::vector<stcp_arpreq>**>(arg);
            ioctl_siocgarpent(tbl);
            break;
        }
        default:
        {
            throw slankdev::exception("invalid arguments");
            break;
        }
    }
}

void arp_module::ioctl_siocaarpent(stcp_arpreq* req)
{
    for (stcp_arpreq& ent : table) {
        if (p_sockaddr_is_same(&ent.arp_pa, &req->arp_pa)) {
            if (hw_sockaddr_is_same(&ent.arp_ha, &req->arp_ha)) {
                return;
            } else {
                for (int i=0; i<6; i++)
                    ent.arp_ha.sa_data[i] = req->arp_ha.sa_data[i];
                return;
            }
        } else { /* ip isnt same */
            continue;
        }
    }
    table.push_back(*req);
}


/* 
 * This functino evaluate only arp_pa and arp_ifindex,
 * because arp_ha is not need deleteing arp-record.
 */
void arp_module::ioctl_siocdarpent(stcp_arpreq* req)
{
    for (size_t i=0; i<table.size(); i++) {
        if (hw_sockaddr_is_same(&req->arp_pa, &table[i].arp_pa) &&
                table[i].arp_ifindex == req->arp_ifindex) {
            table.erase(table.begin() + i);
            return ;
        }
    }
    throw slankdev::exception("arp record not found");
}

void arp_module::ioctl_siocgarpent(std::vector<stcp_arpreq>** tbl)
{
    *tbl = &table;
}


void arp_module::arp_resolv(uint8_t port, const stcp_sockaddr *dst, uint8_t* dsten)
{
    for (stcp_arpreq& req : table) {
        stcp_sockaddr_in* req_in = reinterpret_cast<stcp_sockaddr_in*>(&req.arp_pa);
        const stcp_sockaddr_in* dst_in = reinterpret_cast<const stcp_sockaddr_in*>(dst);
        if (req_in->sin_addr==dst_in->sin_addr && req.arp_ifindex==port) {
            for (int i=0; i<6; i++)
                dsten[i] = req.arp_ha.sa_data[i];
            return;
        }
    }

    if (use_dynamic_arp) {
        throw slankdev::exception("use_dynamic_arp is not impled yet");
    } else {
        throw slankdev::exception("no sush record in arp-table");
    }
}



} /* namespace */
