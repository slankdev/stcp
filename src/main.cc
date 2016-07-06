

#include <stcp/rte.h>
#include <stcp/dpdk.h>

#define BURST_SIZE 32




int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s ...\n", argv[0]);
        return -1;
    }
    // dpdk::core& dpdk = dpdk::core::instance();
    // dpdk.init(argc, argv);
    // dpdk::pkt_queue fifo0;
    // dpdk::pkt_queue fifo1;
    //
    // for (;;) {
    //     for (uint8_t port=0; port<dpdk.num_ports(); port++) {
    //         struct rte_mbuf* bufs[BURST_SIZE];
    //         uint16_t num_rx = dpdk.io_rx(port, bufs, BURST_SIZE);
    //
    //         if (unlikely(num_rx == 0)) continue;
    //         fifo0.enq_array(bufs, num_rx);
    //         printf("port%u recieved %u packets\n", port, num_rx);
    //     }
    //     if (fifo0.size() > 5) break;
    // }
    //
    // printf("before----------------------\n");
    // fifo0.print_info();
    // fifo1.enq(fifo0.deq());
    // fifo1.enq(fifo0.deq());
    //
    //
    // printf("fifo0-----------------------\n");
    // fifo0.print_info();
    //
    //
    // printf("fifo1-----------------------\n");
    // fifo1.print_info();
}



