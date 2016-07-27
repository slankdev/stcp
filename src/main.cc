
#include <stcp/rte.h>
#include <stcp/dpdk.h>
#include <slankdev.h>
#include <vector>
#include <string>
#define BURST_SIZE 32






static void main_recv_loop()
{
    dpdk::core& dpdk = dpdk::core::instance();

    for (dpdk::net_device& dev : dpdk.devices) {
        uint16_t num_rx = dev.io_rx();
        if (num_rx > 0) {
            printf("before refrect ");
            dev.stat();

            struct rte_mbuf* mbuf = rte::pktmbuf_clone(dev.rx.front(),
                    dpdk.get_mempool());
            mbuf->next = nullptr;
            dev.tx.push(mbuf);

            uint16_t num_tx = dev.io_tx(1);
            if (num_tx != 1) {
                printf("tx error 1!=%u\n", num_tx);
            }
            printf("after refrect ");
            dev.stat();
        }
    }
}



int main(int argc, char** argv)
{
    dpdk::core& dpdk = dpdk::core::instance();
    dpdk.init(argc, argv);        
    printf("%zd devices found \n", dpdk.devices.size());
    printf("\033[2J\n"); // clear screen

    for (;;)
        main_recv_loop();
}

