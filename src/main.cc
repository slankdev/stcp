


#include <stcp/dpdk.h>
#include <stcp/rte.h>


// for class stcp
#include <slankdev/log.h>
#include <slankdev/util.h>
#include <slankdev/singleton.h>


class stcp : public slankdev::singleton<stcp> {
    friend slankdev::singleton<stcp>;
    public:
        void init(int argc, char** argv)
        {
            slankdev::log& log = slankdev::log::instance();
            log.open("stcp.log");
            log.push("STCP");

            log.write(slankdev::INFO, "starting...");
            log.write(slankdev::INFO, "starting all inits");

            dpdk& dpdk = dpdk::instance();
            dpdk.init(argc, argv);

            slankdev::clear_screen();
            log.write(slankdev::INFO, "All inits were finished");

            log.pop();
        }
};



static void main_recv_loop()
{
    dpdk& dpdk = dpdk::instance();

    for (net_device& dev : dpdk.devices) {
        uint16_t num_rx = dev.io_rx();
        if (num_rx > 0) {
            printf("before refrect ");
            dev.stat();

            struct rte_mbuf* mbuf = rte::pktmbuf_clone(dev.rx.front(),
                    dpdk.get_mempool());
            mbuf->next = nullptr;
            dev.tx.push(mbuf);
#if 0
            uint16_t num_tx = dev.io_tx(1);
            if (num_tx != 1) {
                printf("tx error 1!=%u\n", num_tx);
            }
            printf("after refrect ");
#endif
            dev.stat();
        }
    }
}



int main(int argc, char** argv)
{
    stcp& s = stcp::instance();
    s.init(argc, argv);
    return 0;
    while (true)
        main_recv_loop();
}

