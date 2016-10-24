

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

class core {
    friend class stcp_app;
private:

public:
    static std::vector<stcp_app*> apps; // should private;

    static udp_module    udp;
    static icmp_module   icmp;
    static ip_module     ip;
    static arp_module    arp;
    static ether_module  ether;
    static dpdk_core     dpdk;

public:
    static void init(int argc, char** argv);
    static void ifs_proc();
    static void run(bool endless=true);
    static void stat_all();

// TODO add this funcs
// public: #<{(| APIs |)}>#
//     static void set_hw_addr();
//     static void set_ip_addr();
//     static void set_default_gw();

private:
    core() = delete;
    core(const core&) = delete;
    core& operator=(const core&) = delete;
};




} /* namespace */
