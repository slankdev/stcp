


#include <stcp/stcp.h>
#include <stcp/socket.h>


namespace slank {


std::vector<stcp_app*> core::apps;
std::vector<stcp_cyclic_func*> core::cyclic_funcs;
tcp_module   core::tcp;
udp_module   core::udp;
icmp_module  core::icmp;
ip_module    core::ip;
arp_module   core::arp;
ether_module core::ether;
dpdk_core    core::dpdk;




stcp_tcp_sock* core::create_tcp_socket()
{
    return tcp.create_socket();
}

void core::destroy_tcp_socket(stcp_tcp_sock* sock)
{
    tcp.destroy_socket(sock);
}


bool core::is_request_to_me(struct stcp_arphdr* ah, uint8_t port) // TODO ERASE
{
	for (ifaddr& ifa : dpdk.devices[port].addrs) {
        stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&ifa.raw);
		if (ifa.family == STCP_AF_INET && sin->sin_addr==ah->pdst)
			return true;
	}
	return false;
}



void core::get_myip(stcp_in_addr* myip, uint8_t port) // TODO ERASE
{
    for (ifaddr& ifa : dpdk.devices[port].addrs) {
        if (ifa.family == STCP_AF_INET) {
            stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&ifa.raw);
            *myip = sin->sin_addr;
            return ;
        }
    }
    throw exception("not found my inet address");
}


void core::get_mymac(stcp_ether_addr* mymac, uint8_t port) // TODO ERASE
{
    for (ifaddr& ifa : dpdk.devices[port].addrs) {
        if (ifa.family == STCP_AF_LINK) {
            for (int i=0; i<6; i++)
                mymac->addr_bytes[i] = ifa.raw.sa_data[i];
            return ;
        }
    }
    throw exception("not found my link address");
}


void core::add_arp_record(
        uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4,
        uint8_t ho1, uint8_t ho2, uint8_t ho3,
        uint8_t ho4, uint8_t ho5, uint8_t ho6)
{
    struct stcp_arpreq req;
    stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&req.arp_pa);

    req.arp_ifindex = 0;
    req.arp_ha = stcp_inet_hwaddr(ho1, ho2, ho3, ho4, ho5, ho6);
    sin->sin_addr = stcp_inet_addr(o1, o2, o3, o4);
    arp.ioctl(STCP_SIOCAARPENT, &req);
}



void core::set_default_gw(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t port)
{
    stcp_rtentry rt;
    rt.rt_gateway.inet_addr(o1, o2, o3, o4);
    rt.rt_port = port;
    ip.ioctl(STCP_SIOCADDGW, &rt);
}



void core::set_ip_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t cidr)
{

    /*
     * Set IP address
     */
    struct stcp_ifreq ifr;
    memset(&ifr, 0, sizeof ifr);
    struct stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&ifr.if_addr);
    sin->sin_addr = stcp_inet_addr(o1, o2, o3, o4);
    dpdk.devices[0].ioctl(STCP_SIOCSIFADDR, &ifr);


    /*
     * Set Netmask
     */
    if (32 < cidr) { // TODO hardcode
        throw exception("out of range");
    }
    union {
        uint8_t u8[4];
        uint32_t u32;
    } U;
    U.u32 = 0xffffffff; // TODO hardcode
    U.u32 >>= (32 - cidr); // TODO hardcode

    memset(&ifr, 0, sizeof ifr);
    sin = reinterpret_cast<stcp_sockaddr_in*>(&ifr.if_addr);
    sin->sin_addr = stcp_inet_addr(U.u8[0], U.u8[1], U.u8[2], U.u8[3]);
    dpdk.devices[0].ioctl(STCP_SIOCSIFNETMASK, &ifr);
}


void core::set_hw_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t o5, uint8_t o6)
{
    struct stcp_ifreq ifr;

    memset(&ifr, 0, sizeof ifr);
    ifr.if_hwaddr.sa_data[0] = o1;
    ifr.if_hwaddr.sa_data[1] = o2;
    ifr.if_hwaddr.sa_data[2] = o3;
    ifr.if_hwaddr.sa_data[3] = o4;
    ifr.if_hwaddr.sa_data[4] = o5;
    ifr.if_hwaddr.sa_data[5] = o6;
    dpdk.devices[0].ioctl(STCP_SIOCSIFHWADDR, &ifr);
}


void core::add_cyclic(stcp_cyclic_func* f)
{
    cyclic_funcs.push_back(f);
}


void core::init(int argc, char** argv)
{
    stat::instance().open_new("stcp.stat.log");
    rxcap::instance().open_new("stcp.rx.log");
    txcap::instance().open_new("stcp.tx.log");
    dmsg::instance().open_new("stcp.dmsg.log");

    dpdk.init(argc, argv);
    ip.init();
}

void core::ifs_proc()
{
    for (ifnet& dev : dpdk.devices) {
        uint16_t num_reqest_to_send = dev.tx_size();
        uint16_t num_tx = dev.io_tx(num_reqest_to_send);

        if (num_tx != num_reqest_to_send) {
            dmsg::instance().write(
                "core::ifs_proc(): num_tx!=num_reqest_to_send, Oh yeah!");
        }

        uint16_t num_rx = dev.io_rx();
        if (unlikely(num_rx == 0)) continue;

        while (dev.rx_size() > 0) {
            mbuf* msg = dev.rx_pop();
            ether.rx_push(msg);
        }
    }
}

void core::run()
{
    uint64_t hz   = rte::get_tsc_hz();
    for (auto cf : cyclic_funcs) {
        cf->prev = rte::get_tsc_cycles();
    }

    core::stat_all(); // TODO ERASE

    // TODO: implement below
    // while (running) {
    while (true) { // statement of while-loop is hardcode.
        uint64_t now = rte::get_tsc_cycles();
        for (auto cf : cyclic_funcs) {
            if (now - cf->prev > cf->interval_ms / 1000.0 * hz) {
                cf->prev = now; // TODO itiguyouiue
                cf->exec();
            }
        }

        for (auto& app : apps) {
            app->proc();
        }

        ifs_proc();
        ether.proc();
        tcp.proc();
        core::stat_all(); // TODO ERASE
    }
}

void core::stat_all()
{
    stat& s = stat::instance();
    s.clean();

    for (ifnet& dev : dpdk.devices) {
        dev.print_stat();
    }

    ether.print_stat();
    s.write("");
    arp.print_stat();
    s.write("");
    ip.print_stat();
    s.write("");
    icmp.print_stat();
    s.write("");
    udp.print_stat();
    s.write("");
    tcp.print_stat();

    s.flush();
}

} /* namespace */
