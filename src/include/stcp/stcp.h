

#pragma once


#include <stcp/dpdk.h>
#include <stcp/config.h>
    
#include <stcp/protocol.h>




class stcp {
private:
    stcp() {}
    ~stcp() {}
    stcp(const stcp&) = delete;
    stcp& operator=(const stcp&) = delete;

public:
    proto_module ether_module;
    proto_module arp_moduele;
    proto_module ip_module;

public:
    static stcp& instance()
    {
        static stcp s;
        return s;
    }

    void init(int argc, char** argv)
    {
        log& log = log::instance();
        log.open("stcp.log");
        log.push("STCP");

        log.write(INFO, "starting...");
        log.write(INFO, "starting all inits");

        dpdk& dpdk = dpdk::instance();
        dpdk.init(argc, argv);

        log.write(INFO, "All inits were finished");

        if (dpdk.devices.size() < 1)
            throw slankdev::exception("this pgm supports 2 or hisgher ports");
        log.pop();
    }

    void run()
    {
        for (size_t i=1; ; i++) {
            lowlayer_loop();

            if (i % 10000 == 0) {
                clear_screen();
                stat_all();
            }
        }
    }

    void lowlayer_loop()
    {
        dpdk& dpdk = dpdk::instance();

        for (size_t i=0; i<dpdk.devices.size(); i++) {
            device_loop(i);
        }
    }

    void device_loop(size_t port_id)
    {
        dpdk& dpdk = dpdk::instance();
        net_device& dev = dpdk.devices[port_id];

        uint16_t num_rx = dev.io_rx();
        if (unlikely(num_rx == 0)) return;

        uint16_t num_reqest_to_send = dev.tx.size();
        uint16_t num_tx = dev.io_tx(num_reqest_to_send);
        if (num_tx != num_reqest_to_send)
            fprintf(stderr, "some packet droped \n");

    }

    void stat_all()
    {
        dpdk& dpdk = dpdk::instance();

        for (net_device& dev : dpdk.devices) {
            printf("%s: ", dev.name.c_str());
            if (dev.promiscuous_mode) printf("PROMISC ");
            printf("\n");
            printf("\tRX Packets %u Queue %zu\n", dev.rx_packets, dev.rx.size());
            printf("\tTX Packets %u Queue %zu\n", dev.tx_packets, dev.tx.size());
            printf("\n");
        }
    }
};


