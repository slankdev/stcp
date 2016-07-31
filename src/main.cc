


#include <stcp/dpdk.h>
#include <stcp/rte.h>
#include <stcp/stcp.h>




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
            clear_screen();
            dev.stat();
        }
    }
}



int main(int argc, char** argv)
{
    stcp& s = stcp::instance();
    s.init(argc, argv);
#if 0
    return 0;
#endif
    while (true)
        main_recv_loop();
}

