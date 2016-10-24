

#pragma once

#include <stdlib.h>

#include <stcp/dpdk.h>
#include <stcp/config.h>
#include <stcp/mbuf.h>
    
#include <stcp/ethernet.h>
#include <stcp/arp.h>
#include <stcp/ip.h>
#include <stcp/icmp.h>
#include <stcp/udp.h>
#include <stcp/app.h>

#include <vector>


namespace slank {
    

using stat  = log<class status_infos>;
using rxcap = log<class rx_packet_log>;
using txcap = log<class tx_packet_log>;
using dmsg  = log<class debug_message_log>;

class stcp_app;



class stcp_cyclic_func {
public:
    uint64_t prev;
    uint64_t interval_ms;
    stcp_cyclic_func(uint64_t ms) : interval_ms(ms) {}
    virtual void exec() = 0;
};


class core {
    friend class stcp_app;

    friend class stcp_udp_sock;

    friend class ifnet;
    friend class ether_module;
    friend class arp_module;
    friend class ip_module;
    friend class icmp_module;
    friend class udp_module;
private:
    static std::vector<stcp_app*> apps; // should private;
    static std::vector<stcp_cyclic_func*> cyclic_funcs;

private:
    static udp_module    udp;
    static icmp_module   icmp;
    static ip_module     ip;
    static arp_module    arp;
    static ether_module  ether;
    static dpdk_core     dpdk;

public:
    static void init(int argc, char** argv);
    static void add_cyclic(stcp_cyclic_func* f);
    static void run();

private:
    static void ifs_proc();
    static void stat_all();
    
public:

    /* 
     * APIs 
     */
    static void set_hw_addr(
            uint8_t o1, uint8_t o2, uint8_t o3, 
            uint8_t o4, uint8_t o5, uint8_t o6);
    static void set_ip_addr(
            uint8_t o1, uint8_t o2, uint8_t o3, 
            uint8_t o4, uint8_t cidr);
    static void set_default_gw(
            uint8_t o1, uint8_t o2, uint8_t o3, 
            uint8_t o4, uint8_t port);
    static void add_arp_record(
            uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, 
            uint8_t ho1, uint8_t ho2, uint8_t ho3, 
            uint8_t ho4, uint8_t ho5, uint8_t ho6);
    static void get_mymac(stcp_ether_addr* mymac, uint8_t port);
    static void get_myip(stcp_in_addr* myip, uint8_t port);
    static bool is_request_to_me(struct stcp_arphdr* ah, uint8_t port);


private:
    core() = delete;
    core(const core&) = delete;
    core& operator=(const core&) = delete;
};




} /* namespace */
