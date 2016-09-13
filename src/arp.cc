

#include <arpa/inet.h>

#include <stcp/arp.h>
#include <stcp/config.h>
#include <stcp/dpdk.h>
#include <stcp/rte.h>

#include <pgen2.h>


namespace slank {
    


static char* ip2cstr(const struct stcp_ip_addr ip)
{
    static char str[16];
    sprintf(str, "%d.%d.%d.%d", 
            ip.addr_bytes[0], ip.addr_bytes[1],
            ip.addr_bytes[2], ip.addr_bytes[3]);
    return str;
}

static char* mac2cstr(const struct ether_addr mac)
{
    static char str[32];
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", 
            mac.addr_bytes[0], mac.addr_bytes[1],
            mac.addr_bytes[2], mac.addr_bytes[3],
            mac.addr_bytes[4], mac.addr_bytes[5]);
    return str;
}

static bool is_same(struct ether_addr& a, struct ether_addr& b)
{
    for (int i=0; i<6; i++) 
        if (a.addr_bytes[i] != b.addr_bytes[i])
            return false;
    return true;
}

void arp_module::stat() 
{
    m.stat();
    printf("\n");
    printf("\tARP-chace\n");
    printf("\t%-16s %-20s %s\n", "Address", "HWaddress", "Iface");
    for (size_t i=0; i<table.size(); i++) {
        for (arpentry& a : table[i].entrys)
            printf("\t%-16s %-20s %zd\n", ip2cstr(a.ip), mac2cstr(a.mac), i);
    }
}

void arp_module::proc() 
{
    while (m.rx_size() > 0) {
        struct rte_mbuf* msg = m.rx_pop();
        struct stcp_arphdr* ah  = rte::pktmbuf_mtod<struct stcp_arphdr*>(msg);
        uint8_t port = msg->port;

        if (ah->operation == htons(2)) { // TODO hard code
            proc_update_arptable(ah, port);
        } else if (ah->operation == htons(1)) {
            proc_arpreply(ah, port);
        }
        rte::pktmbuf_free(msg);
    }

    while (m.tx_size() > 0) throw slankdev::exception("Not Impl yet");
}

void arp_module::proc_update_arptable(struct stcp_arphdr* ah, uint8_t port)
{
    arpentry newent(ah->psrc, ah->hwsrc);

    for (arpentry& ent : table[port].entrys) {
        if (ent.ip == newent.ip) {
            if (is_same(ent.mac, newent.mac)) {
                return;
            } else {
                ent.mac = newent.mac;
                return;
            }
        } else { /* ip isnt same */
            continue;
        }
    }
    table[port].entrys.push_back(newent);
}

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

static bool is_request_to_me(struct stcp_arphdr* ah, uint8_t port)
{
	ifnet& dev = dpdk::instance().devices[port];
	for (ifaddr& ifa : dev.addrs) {
		if (ifa.family == STCP_AF_INET && ifa.raw.in==ah->pdst)
			return true;
	}
	return false;
}

void arp_module::proc_arpreply(struct stcp_arphdr* ah, uint8_t port)
{
	if (is_request_to_me(ah, port)) {
		struct rte_mbuf* msg = alloc_reply_packet(ah, port);

		dpdk& d = dpdk::instance();
		d.devices[port].tx_push(msg);
	}
}




} /* namespace */
