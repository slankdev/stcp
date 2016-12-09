

#pragma once

#include <stdlib.h>

#include <stcp/dataplane.h>
#include <stcp/config.h>
#include <stcp/mbuf.h>

#include <stcp/protos/ethernet.h>
#include <stcp/protos/arp.h>
#include <stcp/protos/ip.h>
#include <stcp/protos/icmp.h>
#include <stcp/protos/udp.h>
#include <stcp/protos/tcp.h>

#include <vector>


namespace stcp {



class tcp_module;
class stcp_tcp_sock;



#if 0
class stcp_cyclic_func {
public:
    uint64_t prev;
    uint64_t interval_ms;
    stcp_cyclic_func(uint64_t ms) : interval_ms(ms) {}
    virtual void exec() = 0;
};
#endif


using stcp_usrapp = int (*)(void*);
struct stcp_usrapp_info {
    stcp_usrapp func;
    void* func_arg;
    uint16_t lcore_id;
};



class core {
    /*
     * TODO XXX delete friend code
     */

    friend class stcp_udp_sock;
    friend class stcp_tcp_sock;

    friend class ifnet;
    friend class ether_module;
    friend class arp_module;
    friend class ip_module;
    friend class icmp_module;
    friend class udp_module;
    friend class tcp_module;

private:
    static std::vector<stcp_usrapp_info> lapps;

public:
    static stcp_tcp_sock* create_tcp_socket();
    static stcp_udp_sock* create_udp_socket();
    static void destroy_tcp_socket(stcp_tcp_sock* sock);
    static void destroy_udp_socket(stcp_udp_sock* sock);

private:
    static tcp_module    tcp;
    static udp_module    udp;
    static icmp_module   icmp;
    static ip_module     ip;
    static arp_module    arp;
    static ether_module  ether;
    static dataplane     dplane;

public:
    static void init(int argc, char** argv);
    static void run();
#if 0
    static void add_cyclic(stcp_cyclic_func* f);
#endif

private:
    static void ifs_proc();
    static void stat_all();

public:

    /*
     * APIs
     */
    static void set_default_gw(
            uint8_t o1, uint8_t o2, uint8_t o3,
            uint8_t o4, uint8_t port);
    static void set_app(stcp_usrapp func_ptr, void* func_arg);

    /*
     * TODO XXX Not Support Multi Interface
     */
    static void set_hw_addr(
            uint8_t o1, uint8_t o2, uint8_t o3,
            uint8_t o4, uint8_t o5, uint8_t o6);
    static void set_ip_addr(
            uint8_t o1, uint8_t o2, uint8_t o3,
            uint8_t o4, uint8_t cidr);
    static void add_arp_record(
            uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4,
            uint8_t ho1, uint8_t ho2, uint8_t ho3,
            uint8_t ho4, uint8_t ho5, uint8_t ho6);
    static void get_mymac(stcp_ether_addr* mymac, uint8_t port);
    static void get_myip(stcp_in_addr* myip, uint8_t port);

    /*
     * TODO rename
     */
    static bool is_request_to_me(struct stcp_arphdr* ah, uint8_t port);


private:
    core() = delete;
    core(const core&) = delete;
    core& operator=(const core&) = delete;
};




} /* namespace */
