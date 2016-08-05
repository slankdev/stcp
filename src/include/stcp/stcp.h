

#pragma once


#include <stcp/dpdk.h>
#include <stcp/config.h>
    
#include <stcp/protocol.h>
#include <stcp/arp.h>
#include <stcp/ip.h>




class stcp {
public:
    arp_module arp;
    ip_module  ip;
    bool modules_updated;

private:
    stcp() : modules_updated(false)
    {
        log& log = log::instance();
        log.open("log.log");
        log.push("STCP");
    }
    ~stcp()
    {
        log& log = log::instance();
        log.pop();
    }
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
        log& log = log::instance();
        log.write(INFO, "init...");

        dpdk& dpdk = dpdk::instance();
        dpdk.init(argc, argv);
        arp.init();
        ip.init();

        user_setting();
    }
    void user_setting();
    void run()
    {
        log& log = log::instance();
        log.write(INFO, "starting STCP...");

        stat_all();

        while (true) {
            modules_updated = false;

            ifs_proc();
            arp.proc();
            ip.proc();

            if (modules_updated)
                stat_all();
        }
    }
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
    void ifs_proc();
};


