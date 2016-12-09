


#include <stcp/stcp.h>
#include <stcp/socket.h>
#include <stcp/filefd.h>
#include <stcp/debug.h>


namespace stcp {


const stcp_ether_addr stcp_ether_addr::broadcast(0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
const stcp_ether_addr stcp_ether_addr::zero(0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
const stcp_in_addr    stcp_in_addr::broadcast(0xff, 0xff, 0xff, 0xff);
const stcp_in_addr    stcp_in_addr::zero(0x00, 0x00, 0x00, 0x00);


std::vector<stcp_usrapp_info> core::lapps;
tcp_module   core::tcp;
udp_module   core::udp;
icmp_module  core::icmp;
ip_module    core::ip;
arp_module   core::arp;
ether_module core::ether;
dataplane    core::dplane;

ncurses      core::screen;
filefd       core::stcp_stdout;
filefd       core::stcp_stddbg;



static int usrapp_wrap(void* arg)
{
    stcp_usrapp_info* app_info = reinterpret_cast<stcp_usrapp_info*>(arg);
    stcp_usrapp f_ptr = app_info->func;
    void*       f_arg = app_info->func_arg;
    try {
        int ret = f_ptr(f_arg);
        stcp_printf("USRREMOTEAPP returned %d\n", ret);

        return ret;
    } catch (std::exception& e) {
        throw exception("User Remote App threw exception!!!");
    }
}



void core::set_app(stcp_usrapp func_ptr, void* func_arg)
{
    static uint32_t c = 0;

    stcp_usrapp_info a;
    a.lcore_id = ++c;
    a.func = func_ptr;
    a.func_arg  = func_arg;

    core::lapps.push_back(a);
    stcp_printf("SET APP lcore_id=%u\n", a.lcore_id);
}





stcp_tcp_sock* core::create_tcp_socket()
{
    for (stcp_tcp_sock& s : tcp.socks) {
        if (s.sock_state == SOCKS_UNUSE) {
            s.init();
            s.sock_state = SOCKS_USE;
            return &s;
        }
    }

    /*
     * TODO reserve socks.
     */
    throw exception("NO SOCKET SPACE");
}

void core::destroy_tcp_socket(stcp_tcp_sock* sock)
{
    sock->term();
    sock->sock_state = SOCKS_UNUSE;
}


stcp_udp_sock* core::create_udp_socket()
{
    stcp_udp_sock* s = new stcp_udp_sock;
    udp.socks.push_back(s);
    return s;
}

void core::destroy_udp_socket(stcp_udp_sock* sock)
{
    for (size_t i=0; i<udp.socks.size(); i++) {
        if (sock == udp.socks[i]) {
            udp.socks.erase(udp.socks.begin() + i);
            return;
        }
    }
    throw exception("OKASHIII");
}



bool core::is_request_to_me(struct stcp_arphdr* ah, uint8_t port) // TODO ERASE
{
	for (ifaddr& ifa : dplane.devices[port].addrs) {
        stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&ifa.raw);
		if (ifa.family == STCP_AF_INET && sin->sin_addr==ah->pdst)
			return true;
	}
	return false;
}



void core::get_myip(stcp_in_addr* myip, uint8_t port) // TODO ERASE
{
    for (ifaddr& ifa : dplane.devices[port].addrs) {
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
    for (ifaddr& ifa : dplane.devices[port].addrs) {
        if (ifa.family == STCP_AF_LINK) {
            for (size_t i=0; i<stcp_ether_addr::addrlen; i++)
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

    req.arp_ifindex = 0;
    req.arp_ha.inet_hwaddr(ho1, ho2, ho3, ho4, ho5, ho6);
    req.arp_pa.inet_addr(o1, o2, o3, o4);
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
    sin->sin_addr.set(o1, o2, o3, o4);
    dplane.devices[0].ioctl(STCP_SIOCSIFADDR, &ifr);


    /*
     * Set Netmask
     */
    uint8_t num_bit = stcp_in_addr::addrlen * 8;
    if (num_bit < cidr) {
        throw exception("out of range");
    }
    union {
        uint8_t u8[4];
        uint32_t u32;
    } U;
    U.u32 = 0xffffffff;
    U.u32 >>= (num_bit - cidr);

    memset(&ifr, 0, sizeof ifr);
    sin = reinterpret_cast<stcp_sockaddr_in*>(&ifr.if_addr);
    sin->sin_addr.set(U.u8[0], U.u8[1], U.u8[2], U.u8[3]);
    dplane.devices[0].ioctl(STCP_SIOCSIFNETMASK, &ifr);
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
    dplane.devices[0].ioctl(STCP_SIOCSIFHWADDR, &ifr);
}


void core::init(int argc, char** argv)
{
    stcp_stdout.fopen("stdout.log", "w");
    stcp_stddbg.fopen("stddbg.log", "w");

    dplane.init(argc, argv);
    arp.init();
    ip.init();
    tcp.init();
}

void core::ifs_proc()
{
    for (ifnet& dev : dplane.devices) {
        uint16_t num_reqest_to_send = dev.tx_size();
        uint16_t num_tx = dev.io_tx(num_reqest_to_send);

        if (num_tx != num_reqest_to_send) {
            throw exception("core::ifs_proc(): num_tx!=num_reqest_to_send, Oh yeah!");
        }

        uint16_t num_rx = dev.io_rx();
        if (unlikely(num_rx == 0)) continue;

        while (!dev.rx_empty()) {
            mbuf* msg = dev.rx_pop();
            ether.rx_push(msg);
        }
    }
}

void core::run()
{
    screen.init();

    for (stcp_usrapp_info& app : lapps) {
        rte::eal_remote_launch(
                usrapp_wrap, reinterpret_cast<void*>(&app), app.lcore_id);
    }

    while (true) {
        ifs_proc();
        ether.proc();
        tcp.proc();
        udp.proc();

        core::stat_all(); // TODO ERASE
    }
}

void core::stat_all()
{
    dplane.print_stat();

    arp.print_stat();
    ip.print_stat();
    icmp.print_stat();
    udp.print_stat();
    tcp.print_stat();

    screen.refresh();
}


} /* namespace */
