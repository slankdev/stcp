


#include <stcp/protos/arp.h>
#include <stcp/config.h>
#include <stcp/stcp.h>
#include <stcp/util.h>
#include <stcp/mempool.h>
#include <stcp/arch/dpdk/device.h>
#include <stcp/tuning.h>


namespace slank {




void arp_module::init()
{
    mp = pool_create(
            "ARP Mem Pool",
            ST_ARPMODULE_MEMPOOL_NSEG * eth_dev_count(),
            ST_ARPMODULE_MP_CACHESIZ,
            0,
            MBUF_DEFAULT_BUF_SIZE, // TODO KOKOKARA MARKED
            cpu_socket_id());
}



void arp_module::rx_push(mbuf* msg)
{

struct stcp_arphdr* ah  = mbuf_mtod<struct stcp_arphdr*>(msg);
uint8_t port = msg->port;

if (ah->operation == hton16(ARPOP_REPLY)) {

    /*
     * Proc ARP-Reply Packet
     * using ARP table.
     */

    stcp_sockaddr     sa_pa(STCP_AF_INET);
    stcp_sockaddr     sa_ha(STCP_AF_LINK);
    stcp_sockaddr_in *sin_pa = reinterpret_cast<stcp_sockaddr_in*>(&sa_pa);

    sin_pa->sin_addr = ah->psrc;
    for (size_t i=0; i<stcp_ether_addr::addrlen; i++)
            sa_ha.sa_data[i] = ah->hwsrc.addr_bytes[i];

        stcp_arpreq req(&sa_pa, &sa_ha, port);
        ioctl_siocaarpent(&req);
        mbuf_free(msg);

    } else if (ah->operation == hton16(ARPOP_REQUEST)) {
        if (core::is_request_to_me(ah, port)) { // TODO

            /*
             * Reply ARP-Reply Packet
             */

            mbuf* msg = mbuf_alloc(mp);
            msg->data_len = sizeof(stcp_arphdr);
            msg->pkt_len  = sizeof(stcp_arphdr);
            msg->port = port;

            stcp_arphdr* rep_ah = mbuf_mtod<stcp_arphdr*>(msg);
            rep_ah->hwtype = hton16(0x0001);
            rep_ah->ptype  = hton16(0x0800);
            rep_ah->hwlen  = static_cast<uint8_t>(stcp_ether_addr::addrlen);
            rep_ah->plen   = static_cast<uint8_t>(stcp_in_addr::addrlen   );
            rep_ah->operation = hton16(ARPOP_REPLY);
            core::get_mymac(&rep_ah->hwsrc, port); // TODO
            rep_ah->psrc  = ah->pdst;
            rep_ah->hwdst = ah->hwsrc;
            rep_ah->pdst  = ah->psrc;

            msg->port = port;
            tx_push(msg);
        } else {
            mbuf_free(msg);
        }
    }
}



void arp_module::tx_push(mbuf* msg)
{
    stcp_sockaddr sa(STCP_AF_ARP);
    core::ether.tx_push(msg->port, msg, &sa);
}


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
        case STCP_SIOCSDARP:
        {
            const bool* b = reinterpret_cast<const bool*>(arg);
            ioctl_siocsdarp(b);
            break;
        }
        case STCP_SIOCGDARP:
        {
            bool* b = reinterpret_cast<bool*>(arg);
            ioctl_siocgdarp(b);
            break;
        }
        default:
        {
            std::string errstr = "invalid arguments " + std::to_string(request);
            throw exception(errstr.c_str());
            break;
        }
    }
}

/*
 * IOCTL SocketIO Add ARP Entry
 *
 * Description
 * This function evaluate only these variable of stcp_arpreq.
 *  - req->arp_ha
 *  - req->arp_ha.sa_data[0-6]
 */
void arp_module::ioctl_siocaarpent(stcp_arpreq* req)
{
    for (stcp_arpreq& ent : table) {
        if (ent.arp_pa == req->arp_pa) {
            if (ent.arp_ha == req->arp_ha) {
                return;
            } else {
                for (size_t i=0; i<stcp_ether_addr::addrlen; i++)
                    ent.arp_ha.sa_data[i] = req->arp_ha.sa_data[i];
                return;
            }
        } else {
            continue;
        }
    }
    table.push_back(*req);
}


/*
 * IOCTL SocketIO Delete ARP Entry
 *
 * Description
 * This functino evaluate only these variables of stcp_arpreq.
 *  - arp_ifindex
 */
void arp_module::ioctl_siocdarpent(stcp_arpreq* req)
{
    for (size_t i=0; i<table.size(); i++) {
        if (req->arp_pa == table[i].arp_pa &&
                table[i].arp_ifindex == req->arp_ifindex) {
            table.erase(table.begin() + i);
            return ;
        }
    }
    throw exception("arp record not found");
}


/*
 * IOCTL SocketIO Get ARP Entrys
 *
 * Description
 *  User can get raw-arptable's pointer. and it is no-const pointer.
 *  But now, I don't assume unsafe operation to arptable's pointer.
 *  So this function's argument will be const pointer to be safety.
 */
void arp_module::ioctl_siocgarpent(std::vector<stcp_arpreq>** tbl)
{
    *tbl = &table;
}


void arp_module::ioctl_siocsdarp(const bool* b)
{
    use_dynamic_arp = *b;
}
void arp_module::ioctl_siocgdarp(bool* b)
{
    *b = use_dynamic_arp;
}



bool arp_module::arp_resolv(
        uint8_t port, const stcp_sockaddr *dst, stcp_ether_addr* dsten, bool checkcacheonly)
{
    const stcp_sockaddr_in* dst_in = reinterpret_cast<const stcp_sockaddr_in*>(dst);
    for (stcp_arpreq& req : table) {
        stcp_sockaddr_in* req_in = reinterpret_cast<stcp_sockaddr_in*>(&req.arp_pa);
        if (req_in->sin_addr==dst_in->sin_addr && req.arp_ifindex==port) {
            for (size_t i=0; i<stcp_ether_addr::addrlen; i++)
                dsten->addr_bytes[i] = req.arp_ha.sa_data[i];
            return true;
        }
    }
    if (checkcacheonly) {
        for (size_t i=0; i<stcp_ether_addr::addrlen; i++)
            dsten->addr_bytes[i] = 0x00;
        return false;
    }

    if (use_dynamic_arp) {
        arp_request(port, &dst_in->sin_addr);
        for (size_t i=0; i<stcp_ether_addr::addrlen; i++)
            dsten->addr_bytes[i] = 0x00;
        return false;
    } else {
        throw exception("no such record in arp-table");
    }
}




void arp_module::arp_request(uint8_t port, const stcp_in_addr* tip)
{
    mbuf* msg = mbuf_alloc(mp);
    msg->data_len = sizeof(stcp_arphdr);
    msg->pkt_len  = sizeof(stcp_arphdr);
    msg->port = port;

    stcp_arphdr* req_ah = mbuf_mtod<stcp_arphdr*>(msg);
    req_ah->hwtype = hton16(0x0001);
	req_ah->ptype  = hton16(0x0800);
	req_ah->hwlen  = static_cast<uint8_t>(stcp_ether_addr::addrlen);
	req_ah->plen   = static_cast<uint8_t>(stcp_in_addr::addrlen   );
    req_ah->operation = hton16(ARPOP_REQUEST);
    core::get_mymac(&req_ah->hwsrc, port); // TODO
    core::get_myip(&req_ah->psrc, port); // TODO
    for (size_t i=0; i<stcp_ether_addr::addrlen; i++) {
        req_ah->hwdst.addr_bytes[i] = 0x00;
    }
    req_ah->pdst = *tip;
    tx_push(msg);
}

void arp_module::print_stat() const
{
    size_t rootx = screen.POS_ARP.x;
    size_t rooty = screen.POS_ARP.y;
    screen.move(rooty, rootx);

    screen.printwln("ARP module");
    screen.printwln(" Pool: %u/%u", pool_use_count(mp), pool_size(mp));
    screen.printwln(" Waiting packs  : %zd", arpresolv_wait_queue.size());
    screen.printwln(" Use dynamic arp: %s", use_dynamic_arp ? "YES" : "NO");
    screen.printwln(" ARP-chace");
    screen.printwln(" %-16s %-20s %s", "Address", "HWaddress", "Iface");

    size_t i=0;
    for (const stcp_arpreq& a : table) {
        std::string pa = a.arp_pa.c_str();
        std::string ha = a.arp_ha.c_str();
        screen.printwln(" %-16s %-20s %d",
                pa.c_str(),
                ha.c_str(),
                a.arp_ifindex);
        i++;
    }
}


} /* namespace */
