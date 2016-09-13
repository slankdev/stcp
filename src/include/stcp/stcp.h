

#pragma once


#include <stcp/dpdk.h>
#include <stcp/config.h>
    
#include <stcp/protocol.h>
#include <stcp/arp.h>
#include <stcp/ip.h>


namespace slank {
    


class core {
public:
    arp_module arp;
    ip_module  ip;
    bool modules_updated;

private:
    core() : modules_updated(false) {}
    ~core() {}
    core(const core&) = delete;
    core& operator=(const core&) = delete;

public:
    static core& instance()
    {
        static core s;
        return s;
    }
    void init(int argc, char** argv)
    {
        dpdk& dpdk = dpdk::instance();
        dpdk.init(argc, argv);
        arp.init();
        ip.init();
    }
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
