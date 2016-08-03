

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

private:
    stcp()
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

    void init(int argc, char** argv);
    void run();
    void ifs_proc();
    void stat_all();
};


