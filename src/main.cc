


#include <stcp/rte.h>
#include <stcp/dpdk.h>


// for class stcp
#include <slankdev/log.h>
#include <slankdev/util.h>
#include <slankdev/singleton.h>


class stcp {
    private:
    public:
        stcp(int argc, char** argv)
        {
            dpdk& dpdk = dpdk::instance();
            slankdev::log& log = slankdev::singleton<slankdev::log>::instance();

            log.open("stcp.log");
            dpdk.init(argc, argv);

            slankdev::clear_screen();
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

            // uint16_t num_tx = dev.io_tx(1);
            // if (num_tx != 1) {
            //     printf("tx error 1!=%u\n", num_tx);
            // }
            // printf("after refrect ");
            dev.stat();
        }
    }
}



int main(int argc, char** argv)
{
    stcp s(argc, argv);

    for (;;)
        main_recv_loop();
}

