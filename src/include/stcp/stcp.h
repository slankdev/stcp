

#pragma once

#include <stdlib.h>

#include <stcp/dpdk.h>
#include <stcp/config.h>
#include <stcp/mbuf.h>
    
#include <stcp/protocol.h>
#include <stcp/ethernet.h>
#include <stcp/arp.h>
#include <stcp/ip.h>


namespace slank {
    



class core {
public:
    ip_module  ip;
    arp_module arp;
    ether_module ether;
    bool modules_updated;
    dpdk_core dpdk;

private:
    core() :  ether(arp, ip), modules_updated(false) {}
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
        dpdk.init(argc, argv);
        arp.init();
        ip.init();
    }
    void ifs_proc()
    {
        for (ifnet& dev : dpdk.devices) {
            uint16_t num_reqest_to_send = dev.tx_size();
            uint16_t num_tx = dev.io_tx(num_reqest_to_send);
            if (num_tx != num_reqest_to_send)
                throw slankdev::exception("some packet droped");

            uint16_t num_rx = dev.io_rx();
            if (unlikely(num_rx == 0)) continue;

            modules_updated = true;
            while (dev.rx_size() > 0) {
                struct rte_mbuf* msg = dev.rx_pop();
                ether.rx_push(msg);
            }
        }
    }

    void run()
    {
        stat_all();
        while (true) {
            modules_updated = false;

            ifs_proc();
            ether.proc();
            arp.proc();
            ip.proc();


            if (modules_updated)
                stat_all();
        }
    }


    void stat_all()
    {
        clear_screen();

        for (ifnet& dev : dpdk.devices) {
            dev.stat();
        }

        ether.stat();
        arp.stat();
        ip.stat();
    }
};




} /* namespace */
