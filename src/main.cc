

#include <stcp/rte.h>
#include <stcp/dpdk.h>
#include <slankdev.h>
#include <vector>
#include <string>
#define BURST_SIZE 32


class myallocator {
    public:
        void deallocate(struct rte_mbuf* ptr)
        {
            rte::pktmbuf_free(ptr);
        }
};


static void analyze(uint8_t port, slankdev::queue<struct rte_mbuf, myallocator>& q)
{

    while (q.size() > 0) {
        struct rte_mbuf* mbuf = q.pop();

        if(rte::mbuf2len(mbuf) < 14) {
            printf("length: %zd\n", rte::mbuf2len(mbuf));
            continue ;
        }

        rte::pktmbuf_free(mbuf);
        printf("%d: recv \n", port);
    }
}



int main(int argc, char** argv)
{
    dpdk::core& dpdk = dpdk::core::instance();
    dpdk.init(argc, argv);
    slankdev::queue<struct rte_mbuf, myallocator> q;

    printf("%zd devices found \n", dpdk.devices.size());

    for (;;) {
        for (uint8_t port=0; port<dpdk.devices.size(); port++) {
            struct rte_mbuf* bufs[BURST_SIZE];
            uint16_t num_rx = dpdk.io_rx(port, bufs, BURST_SIZE);

            if (unlikely(num_rx == 0)) continue;
            q.push(dpdk::array2llist_mbuf(bufs, num_rx));
            analyze(port, q);
        }
    }
}
