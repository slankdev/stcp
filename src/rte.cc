


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#include <rte_config.h>
#include <rte_version.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_hexdump.h>
#include <rte_ip.h>

#include <stcp/rte.h>
#include <stcp/stcp.h>



namespace slank {
    

} /* namespace */


namespace rte {
    


void eth_dev_init(int argc, char** argv)
{
    int ret = rte_eal_init(argc, argv);
    if (ret < 0) {
        throw rte::exception("rte_eal_init");
    }
}

void eth_dev_configure(uint8_t port_id, uint16_t nb_rx_queue,
        uint16_t nb_tx_queue, const struct rte_eth_conf* eth_conf)
{
    int ret = rte_eth_dev_configure(port_id, nb_rx_queue, nb_tx_queue, eth_conf);
    if (ret != 0) {
        throw rte::exception("eth_eth_configure");
    }
}

void eth_rx_queue_setup(uint8_t port_id, uint16_t rx_queue_id, 
        uint16_t nb_rx_desc, uint32_t socket_id, 
        const struct rte_eth_rxconf* rx_conf, struct rte_mempool* mbuf_pool)
{
    int ret = rte_eth_rx_queue_setup(port_id, rx_queue_id, 
            nb_rx_desc, socket_id, rx_conf, mbuf_pool);
    if (ret < 0) {
        throw rte::exception("rx_queue_setup");
    }
}

void eth_tx_queue_setup(uint8_t port_id, uint16_t tx_queue_id, 
        uint16_t nb_tx_desc, uint32_t socket_id, 
        const struct rte_eth_txconf* tx_conf)
{
    int ret = rte_eth_tx_queue_setup(port_id, tx_queue_id, 
            nb_tx_desc, socket_id, tx_conf);
    if (ret < 0) {
        throw rte::exception("tx_queue_setup");
    }
}

void eth_dev_start(uint8_t port_id)
{
    int ret = rte_eth_dev_start(port_id);
    if (ret < 0) {
        throw rte::exception("rte_eth_dev_start");
    }
}

void eth_promiscuous_enable(uint8_t port_id)
{
    rte_eth_promiscuous_enable(port_id);
}

void eth_promiscuous_disable(uint8_t port_id)
{
    rte_eth_promiscuous_disable(port_id);
}

int eth_promiscuous_get(uint8_t port_id)
{
    return rte_eth_promiscuous_get(port_id);
}

struct rte_mempool* pktmbuf_pool_create(const char* name, uint32_t n, 
        uint32_t cache_size, uint16_t priv_size, 
        uint16_t data_room_size, int socket_id)
{
    rte_mempool* mbuf_pool = rte_pktmbuf_pool_create(
            name, n, cache_size, priv_size, data_room_size, socket_id);
    if (mbuf_pool == NULL)
        rte_exit(EXIT_FAILURE, "pktmbuf_pool_create()");
    return mbuf_pool;
}

size_t eth_dev_count()
{
    return rte_eth_dev_count();
}

size_t lcore_count()
{
    return rte_lcore_count();
}

void eth_macaddr_get(uint8_t port_id, struct ether_addr* mac_addr)
{
    rte_eth_macaddr_get(port_id, mac_addr);
}

uint16_t eth_rx_burst(uint8_t port_id, uint16_t queue_id, 
        rte_mbuf** rx_pkts, const uint16_t nb_pkts)
{
    static uint64_t cnt = 1;
    uint16_t num_rx = rte_eth_rx_burst(port_id, queue_id, rx_pkts, nb_pkts);
    for (uint16_t i=0; i<num_rx; i++) {
        slank::rxcap::instance().write("%lu: recv", cnt);
        cnt++;
        rte::pktmbuf_dump(slank::rxcap::instance().stream()
                , rx_pkts[i], rte::pktmbuf_pkt_len(rx_pkts[i]));
        slank::rxcap::instance().write("");
        slank::rxcap::instance().write("");
    }
    slank::rxcap::instance().flush();
    return num_rx;
}

uint16_t eth_tx_burst(uint8_t port_id, uint16_t queue_id, 
        rte_mbuf** tx_pkts, uint16_t nb_pkts)
{
    static uint64_t cnt = 1;
    for (uint16_t i=0; i<nb_pkts; i++) {
        slank::txcap::instance().write("%lu: send", cnt);
        cnt++;
        rte::pktmbuf_dump(slank::txcap::instance().stream()
                , tx_pkts[i], rte::pktmbuf_pkt_len(tx_pkts[i]));
        slank::txcap::instance().write("");
        slank::txcap::instance().write("");
    }
    slank::txcap::instance().flush();
    uint16_t num_tx = rte_eth_tx_burst(port_id, queue_id, tx_pkts, nb_pkts);
    return num_tx;
}

void pktmbuf_free(rte_mbuf* m)
{
    rte_pktmbuf_free(m);
}

uint32_t socket_id()
{
    return rte_socket_id();
}

void pktmbuf_dump(FILE* f,const rte_mbuf* m, unsigned dump_len)
{
    rte_pktmbuf_dump(f, m, dump_len);
    fprintf(f, "\n\n\n\n");
}

int eth_dev_socket_id(uint8_t port_id)
{
    return rte_eth_dev_socket_id(port_id);
}

rte_mbuf* pktmbuf_alloc(struct rte_mempool* mp)
{
    rte_mbuf* buf = rte_pktmbuf_alloc(mp);
    if (buf == nullptr) {
        throw rte::exception("rte_pktmbuf_alloc");
    }
    return buf;
}

rte_mbuf* pktmbuf_clone(rte_mbuf* md, struct rte_mempool* mp)
{
    rte_mbuf* buf = rte_pktmbuf_clone(md, mp);
    if (buf == nullptr) {
        throw rte::exception("rte_pktmbuf_clone");
    }
    return buf;
}

uint16_t pktmbuf_headroom(const rte_mbuf* m)
{
    return rte_pktmbuf_headroom(m);
}
uint16_t pktmbuf_tailroom(const rte_mbuf* m)
{
    return rte_pktmbuf_tailroom(m);
}
void pktmbuf_prepend(rte_mbuf* m, uint16_t len)
{
    char* ret = rte_pktmbuf_prepend(m, len);
    if (ret == nullptr) {
        throw rte::exception("rte_pktmbuf_prepend");
    }
}
void pktmbuf_append(rte_mbuf* m, uint16_t len)
{
    char* ret = rte_pktmbuf_append(m, len);
    if (ret == nullptr) {
        throw rte::exception("rte_pktmbuf_append");
    }
}
void pktmbuf_adj(rte_mbuf* m, uint16_t len)
{
    char* ret = rte_pktmbuf_adj(m, len);
    if (ret == nullptr) {
        throw rte::exception("rte_pktmbuf_adj");
    }
}
void pktmbuf_trim(rte_mbuf* m, uint16_t len)
{
    int ret = rte_pktmbuf_trim(m, len);
    if (ret == -1) {
        throw rte::exception("rte_pktmbuf_trim");
    }
}

size_t pktmbuf_data_len(const rte_mbuf* m)
{
    return m->data_len;
}

size_t pktmbuf_pkt_len(const rte_mbuf* m)
{
    return m->pkt_len;
}

size_t raw_cksum(const void* buf, size_t len)
{
    return rte_raw_cksum(buf, len);
}

uint16_t bswap16(uint16_t x)
{
    return rte_bswap16(x);
}
uint32_t bswap32(uint32_t x)
{
    return rte_bswap32(x);
}
uint64_t bswap64(uint64_t x)
{
    return rte_bswap64(x);
}

uint32_t ipv4_fragment_packet(rte_mbuf* pkt_in, rte_mbuf** pkts_out, uint16_t nb_pkts_out, 
        uint16_t mtu_size, struct rte_mempool* pool_direct, struct rte_mempool* pool_indirect) noexcept
{
    int32_t res = rte_ipv4_fragment_packet(pkt_in, pkts_out, 
            nb_pkts_out, mtu_size, pool_direct, pool_indirect);
    if (res < 0) {
        return 1;
    }
    return res;
}

void prefetch0(const volatile void* p)
{
    rte_prefetch0(p);
}
void prefetch1(const volatile void* p)
{
    rte_prefetch1(p);
}
void prefetch2(const volatile void* p)
{
    rte_prefetch2(p);
}

bool ipv4_frag_pkt_is_fragmented(const void* iph)
{
    int ret = rte_ipv4_frag_pkt_is_fragmented(
            reinterpret_cast<const ipv4_hdr*>(iph));
    return ret==1 ? true : false;
}



struct rte_mbuf* ipv4_frag_reassemble_packet(
        struct rte_ip_frag_tbl*        tbl,
        struct rte_ip_frag_death_row*  dr,
        struct rte_mbuf*               mb,
        uint64_t                       tms,
        struct ipv4_hdr*               iph    )
{
    return rte_ipv4_frag_reassemble_packet(tbl, dr, mb, tms, iph);
}

} /* namespace rte */
