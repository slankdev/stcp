
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <inttypes.h>

#include <rte_config.h>
#include <rte_version.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_hexdump.h>

#include <stcp/rte.h>
#include <stcp/dpdk.h>






namespace dpdk {



uint16_t core::rx_ring_size = 128;
uint16_t core::tx_ring_size = 512;
uint32_t core::num_mbufs = 8192;
uint32_t core::mbuf_cache_size = 250;
uint16_t core::num_rx_rings = 1;
uint16_t core::num_tx_rings = 1;



core& core::instance()
{
    static core instance;
    return instance;
}

core::core() : mempool(nullptr) {}
core::~core() {}

void core::init(int argc, char** argv)
{
    rte::eth_dev_init(argc, argv);
    
    if (rte::eth_dev_count() < 1) {
        throw rte::exception("num of devices is less than 1");
    }

    mempool = rte::pktmbuf_pool_create(
            "SLANK", 
            num_mbufs * rte::eth_dev_count(), 
            mbuf_cache_size, 
            0, 
            RTE_MBUF_DEFAULT_BUF_SIZE, 
            rte::socket_id()
            );

    for (size_t port=0; port<num_ports(); port++) {
        port_init(port);
    }
}

void core::port_init(uint8_t port)
{

    struct rte_eth_conf port_conf;
    memset(&port_conf, 0, sizeof port_conf);
    port_conf.rxmode.max_rx_pkt_len = ETHER_MAX_LEN;

    rte::eth_dev_configure(port, num_rx_rings, num_tx_rings, &port_conf);

    for (uint16_t ring=0; ring<num_rx_rings; ring++) {
        rte::eth_rx_queue_setup(port, ring, rx_ring_size,
                rte::eth_dev_socket_id(port), NULL, this->mempool); 
    }
    for (uint16_t ring=0; ring<num_tx_rings; ring++) {
        rte::eth_tx_queue_setup(port, ring, tx_ring_size,
                rte::eth_dev_socket_id(port), NULL); 
    }
    rte::eth_dev_start(port);
    rte::eth_promiscuous_enable(port);

    if (rte::eth_dev_socket_id(port) > 0 && 
            rte::eth_dev_socket_id(port) != (int)rte::socket_id()) {
        char str[128];
        sprintf(str, "WARNING: port %4u is on remote NUMA node to "
                "polling thread. \n\tPerformance will "
                "not be optimal. \n ", port);
        throw rte::exception(str);
    }

}

size_t core::num_ports()
{
    return rte::eth_dev_count();
}

uint16_t core::io_rx(uint16_t port, struct rte_mbuf** bufs, size_t num_bufs)
{
    if (port > num_ports())
        throw rte::exception("port number is too large.");

    return rte::eth_rx_burst(port, 0, bufs, num_bufs);
}

uint16_t core::io_tx(uint16_t port, struct rte_mbuf** bufs, size_t num_bufs)
{
    if (port > num_ports())
        throw rte::exception("port number is too large.");

    return rte::eth_tx_burst(port, 0, bufs, num_bufs);
}
 






// FOR DEBUG
// static void print_mbuf_links(const struct rte_mbuf* mbuf)
// {
//     printf("[%p]", mbuf);
//     if (mbuf) {
//         printf("->");
//         print_mbuf_links(mbuf->next);
//     } else {
//         printf("\n");
//     }
// }
// static struct rte_mbuf* array2linklist(struct rte_mbuf** bufs, size_t num_bufs)
// {
//     struct rte_mbuf* link_head = bufs[0];
//     struct rte_mbuf* link = link_head;
//     for (size_t i=0; i<num_bufs-1; i++) {
//         link->next = bufs[i+1];
//         link = link->next;
//     }
//     return link_head;
// }
//


static struct rte_mbuf* get_tail(struct rte_mbuf* head)
{
    if (!head)
        return nullptr;

    while (head->next)
        head = head->next;
    return head;
}


pkt_queue::pkt_queue() : head(nullptr) {}
pkt_queue::~pkt_queue()
{
    struct rte_mbuf* link = head;
    while (link) {
        struct rte_mbuf* next = link->next;
        rte::pktmbuf_free(link);
        link = next;
    }
}

void pkt_queue::enq_array(struct rte_mbuf** bufs, size_t num_bufs)
{
    for (size_t i=0; i<num_bufs; i++) {
        enq(bufs[i]);
    }
}

void pkt_queue::enq(struct rte_mbuf* buf)
{
    struct rte_mbuf* tail = get_tail(head);
    if (tail)
        tail->next = buf;
    else 
        head = buf;
}

struct rte_mbuf* pkt_queue::deq()
{
    struct rte_mbuf* node = head;
    head = node->next;
    node->next = nullptr;
    return node;
}


void pkt_queue::print_info()
{
    printf("%zd packets\n", size());
    struct rte_mbuf* mbuf = head;
    int i=0;
    for (; mbuf; i++) {
        for (int j=0; j<i; j++) printf(" ");
        printf("[%p]\n", mbuf);
        mbuf = mbuf->next;

    }
    for (int j=0; j<i; j++) printf(" ");
    printf("[nil]\n");
}

size_t pkt_queue::size()
{
    size_t cnt = 0;
    struct rte_mbuf* mbuf = head;
    while (mbuf) {
        cnt++;
        mbuf = mbuf->next;
    }
    return cnt;
}







} /* namespace dpdk */


