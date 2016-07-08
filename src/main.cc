

#include <stcp/rte.h>
#include <stcp/dpdk.h>
#include <stcp/proto_module.h>
#include <slankdev.h>

#define BURST_SIZE 32
#define ETHER_TYPE(PTR) *(uint16_t*)((uint8_t*)(PTR)+12)



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




static void analyze(uint8_t port, protocol::proto_module& mod)
{
    info& infos = slankdev::singleton<info>::instance();

    while (mod.size(protocol::rx) > 0) {
        struct rte_mbuf* mbuf = mod.output(protocol::rx);

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

    struct rte_mbuf* bufs[mod.size(protocol::tx)];
    size_t txsize = mod.size(protocol::tx);
    for (size_t i=0; i<txsize; i++) {
        bufs[i] = mod.output(protocol::tx);
    }

    dpdk::core& dpdk = dpdk::core::instance();
    dpdk.io_tx(port, bufs, txsize);
    infos.cnts[TX][OTHER] += txsize;

    infos.print();
}




int main(int argc, char** argv)
{
    dpdk::core& dpdk = dpdk::core::instance();
    dpdk.init(argc, argv);
    protocol::proto_module mod;

    for (;;) {
        for (uint8_t port=0; port<dpdk.num_ports(); port++) {
            struct rte_mbuf* bufs[BURST_SIZE];
            uint16_t num_rx = dpdk.io_rx(port, bufs, BURST_SIZE);

            if (unlikely(num_rx == 0)) continue;
            mod.input(protocol::rx, dpdk::pkt_queue::array2llist(bufs, num_rx));
            analyze(port, mod);
        }
    }
}



