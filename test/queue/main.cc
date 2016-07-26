

#include <stcp/rte.h>
#include <stcp/dpdk.h>
#include <slankdev.h>
#define BURST_SIZE 32
#define ETHER_TYPE(PTR) *(uint16_t*)((uint8_t*)(PTR)+12)



class myallocator {
    public:
        struct rte_mbuf* allocate(size_t size)
        {
            // return rte::pktmbuf_alloc();
            return nullptr;
        }
        void deallocate(struct rte_mbuf* ptr)
        {
            rte::pktmbuf_free(ptr);
        }
};


enum {
    TX=0,
    RX
};
enum {
    ARP=0,
    IP,
    IP6,
    OTHER
};
struct info {
    size_t cnts[2][4];
    size_t cnt_all(int trx) 
    {
        size_t cnt = 0;
        for (size_t i=0; i<4; i++)
            cnt += cnts[trx][i];
        return cnt;
    }
    info()
    {
        memset(cnts, 0, sizeof(cnts));
    }
    void print()
    {
        printf("\033[2J"); // clear screen
        printf("rx: %zd \n", cnt_all(RX));
        printf(" - arp: %zd \n", cnts[RX][ARP]);
        printf(" - ip : %zd \n", cnts[RX][IP] );
        printf(" - ip6: %zd \n", cnts[RX][IP6]);
        printf("tx: %zd \n", cnt_all(TX));
        printf(" - arp: %zd \n", cnts[TX][ARP]);
        printf(" - ip : %zd \n", cnts[TX][IP] );
        printf(" - ip6: %zd \n", cnts[TX][IP6]);
    }
};





static void analyze(uint8_t port, slankdev::queue<struct rte_mbuf, myallocator>& q)
{
    info& infos = slankdev::singleton<info>::instance();

    while (q.size() > 0) {
        struct rte_mbuf* mbuf = q.pop();

        if(rte::mbuf2len(mbuf) < 14) {
            printf("length: %zd\n", rte::mbuf2len(mbuf));
            continue ;
        }

        uint16_t type = slankdev::ntohs( ETHER_TYPE(rte::mbuf2ptr(mbuf)) );
        if (type == 0x0806) {
            infos.cnts[RX][ARP] ++;
        } else if (type == 0x0800) {
            infos.cnts[RX][IP] ++;
        } else if (type == 0x86dd) {
            infos.cnts[RX][IP6] ++;
        } else {
            printf("unknown ether type: 0x%04x \n", type);
            infos.cnts[RX][OTHER] ++;
        }

        rte::pktmbuf_free(mbuf);
    }

    infos.print();
}



int main(int argc, char** argv)
{
    dpdk::core& dpdk = dpdk::core::instance();
    dpdk.init(argc, argv);
    slankdev::queue<struct rte_mbuf, myallocator> q;

    for (;;) {
        for (uint8_t port=0; port<dpdk.num_ports(); port++) {
            struct rte_mbuf* bufs[BURST_SIZE];
            uint16_t num_rx = dpdk.io_rx(port, bufs, BURST_SIZE);

            if (unlikely(num_rx == 0)) continue;
            q.push(dpdk::array2llist_mbuf(bufs, num_rx));
            analyze(port, q);
        }
    }
}








