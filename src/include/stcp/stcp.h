

#pragma once


#include <stcp/dpdk.h>
#include <stcp/config.h>
    
#include <stcp/protocol.h>
#include <stcp/arp.h>
#include <stcp/ip.h>


namespace slank {
    


class stcp {
public:
    arp_module arp;
    ip_module  ip;
    bool modules_updated;

private:
    stcp() : modules_updated(false) {}
    ~stcp() {}
    stcp(const stcp&) = delete;
    stcp& operator=(const stcp&) = delete;

public:
    static stcp& instance()
    {
        static stcp s;
        return s;
    }
    void init(int argc, char** argv)
    {
        dpdk& dpdk = dpdk::instance();
        dpdk.init(argc, argv);
        arp.init();
        ip.init();
    }
    // void user_setting();
    void ifs_proc();
    void run();
    void stat_all()
    {
        dpdk& dpdk = dpdk::instance();
        clear_screen();

        for (ifnet& dev : dpdk.devices) {
            dev.stat();
        }

        arp.stat();
        ip.stat();
    }
};




} /* namespace */
