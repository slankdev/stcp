

#pragma once

#include <stdlib.h>

#include <stcp/dpdk.h>
#include <stcp/config.h>
#include <stcp/mbuf.h>
    
#include <stcp/protocol.h>
#include <stcp/arp.h>
#include <stcp/ip.h>


namespace slank {
    

static uint16_t get_ether_type(struct rte_mbuf* msg)
{
    struct stcp_ether_header* eh;
    eh = rte_pktmbuf_mtod(msg, struct stcp_ether_header*);
    return rte_bswap16(eh->type);
}


class core {
public:
    arp_module arp;
    ip_module  ip;
    bool modules_updated;
    dpdk_core dpdk;

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
        dpdk.init(argc, argv);
        arp.init();
        ip.init();
    }
    void ifs_proc()
    {
        for (ifnet& dev : dpdk.devices) {
            uint16_t num_rx = dev.io_rx();
            if (unlikely(num_rx == 0)) continue;

            modules_updated = true;

            uint16_t num_reqest_to_send = dev.tx_size();
            uint16_t num_tx = dev.io_tx(num_reqest_to_send);
            if (num_tx != num_reqest_to_send)
                fprintf(stderr, "some packet droped \n");

            while (dev.rx_size() > 0) {
                struct rte_mbuf* msg = dev.rx_pop();
                uint16_t etype = get_ether_type(msg);
                mbuf_pull(msg, sizeof(struct stcp_ether_header));

                switch (etype) {
                    case 0x0800:
                    {
                        ip.rx_push(msg);
                        break;
                    }
                    case 0x0806:
                    {
                        arp.rx_push(msg);
                        break;
                    }
                    default:
                    {
                        dev.drop(msg);
                        break;
                    }
                }
            }
        }
    }

    void run()
    {
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
        clear_screen();

        for (ifnet& dev : dpdk.devices) {
            dev.stat();
        }

        arp.stat();
        ip.stat();
    }
};




} /* namespace */
