

#pragma once


#include <stcp/dpdk.h>
#include <stcp/config.h>
    

 
class stcp {
    private:
        stcp() {}
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
            log& log = log::instance();
            log.open("stcp.log");
            log.push("STCP");

            log.write(INFO, "starting...");
            log.write(INFO, "starting all inits");

            dpdk& dpdk = dpdk::instance();
            dpdk.init(argc, argv);

            log.write(INFO, "All inits were finished");

            log.pop();
        }
        void run()
        {
            dpdk& dpdk = dpdk::instance();
            if (dpdk.devices.size() != 2) {
                throw slankdev::exception("this pgm supports only 2 port");
            }

            while (1) {
                for (size_t i=0; i<dpdk.devices.size(); i++) {
                    net_device& recv_dev = dpdk.devices[i];
                    net_device& send_dev = dpdk.devices[i^1];

                    uint16_t num_rx = recv_dev.io_rx();
                    if (unlikely(num_rx == 0)) continue;

                    while (recv_dev.rx.size() > 0) {
                        send_dev.tx.push(recv_dev.rx.pop());
                    }

                    uint16_t num_tx = send_dev.io_tx(num_rx);
                    if (num_rx != num_tx)
                        fprintf(stderr, "some packet droped \n");

                    clear_screen();
                    printf("==============================\n");
                    dpdk.devices[0].stat();
                    dpdk.devices[1].stat();
                    printf("now: %u packets forwarded\n", num_tx);
                    printf("==============================\n");
                }
            }
        }
};


