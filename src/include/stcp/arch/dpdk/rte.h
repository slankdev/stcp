
#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include <string>
#include <iostream>
#include <sstream>
#include <exception>

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
#include <rte_ip_frag.h>


namespace slank {




} /* namespace */


namespace rte {

class exception : public std::exception {
    private:
        std::string str;
    public:
        explicit exception(const char* s="") noexcept {
            // int e = errno;
            str = s;
        }
        template<class T>
        exception& operator<<(const T& t) noexcept {
            std::ostringstream os;
            os << " " << t ;
            str += os.str();
            return *this;
        }
        const char* what() const noexcept {
            return str.c_str();
        }
};

void* mbuf2ptr(struct rte_mbuf* mbuf);
size_t mbuf2len(struct rte_mbuf* mbuf);
void eth_dev_init(int argc, char** argv);
void eth_dev_configure(uint8_t port_id, uint16_t nb_rx_queue,
        uint16_t nb_tx_queue, const struct rte_eth_conf* eth_conf);
void eth_rx_queue_setup(uint8_t port_id, uint16_t rx_queue_id,
        uint16_t nb_rx_desc, uint32_t socket_id,
        const struct rte_eth_rxconf* rx_conf, struct rte_mempool* mbuf_pool);
void eth_tx_queue_setup(uint8_t port_id, uint16_t tx_queue_id,
        uint16_t nb_tx_desc, uint32_t socket_id,
        const struct rte_eth_txconf* tx_conf);
void eth_dev_start(uint8_t port_id);
void eth_promiscuous_enable(uint8_t port_id);
void eth_promiscuous_disable(uint8_t port_id);
int eth_promiscuous_get(uint8_t port_id);
struct rte_mempool* pktmbuf_pool_create(const char* name, uint32_t n,
        uint32_t cache_size, uint16_t priv_size,
        uint16_t data_room_size, int socket_id);
size_t eth_dev_count();
size_t lcore_count();
void eth_macaddr_get(uint8_t port_id, struct ether_addr* mac_addr);
uint16_t eth_rx_burst(uint8_t port_id, uint16_t queue_id,
        struct rte_mbuf** rx_pkts, const uint16_t nb_pkts);
uint16_t eth_tx_burst(uint8_t port_id, uint16_t queue_id,
        struct rte_mbuf** tx_pkts, uint16_t nb_pkts);
void pktmbuf_free(struct rte_mbuf* m);
uint32_t socket_id();
void pktmbuf_dump(FILE* f,const struct rte_mbuf* m, unsigned dump_len);
int eth_dev_socket_id(uint8_t port_id);
struct rte_mbuf* pktmbuf_alloc(struct rte_mempool* mp);
struct rte_mbuf* pktmbuf_clone(struct rte_mbuf* md, struct rte_mempool* mp);

uint16_t pktmbuf_headroom(const struct rte_mbuf* m);
uint16_t pktmbuf_tailroom(const struct rte_mbuf* m);
void pktmbuf_prepend(struct rte_mbuf* m, uint16_t len);
void pktmbuf_append(struct rte_mbuf* m, uint16_t len);
void pktmbuf_adj(struct rte_mbuf* m, uint16_t len);
void pktmbuf_trim(struct rte_mbuf* m, uint16_t len);
size_t pktmbuf_data_len(const struct rte_mbuf* m);
size_t pktmbuf_pkt_len(const struct rte_mbuf* m);
size_t raw_cksum(const void* buf, size_t len);


uint16_t bswap16(uint16_t x);
uint32_t bswap32(uint32_t x);
uint64_t bswap64(uint64_t x);

void prefetch0(const volatile void* p);
void prefetch1(const volatile void* p);
void prefetch2(const volatile void* p);

bool pktmbuf_is_contiguous(const rte_mbuf* m);
void* malloc(const char* type, size_t size, unsigned align);
void free(void* ptr);
void* memcpy(void* dst, const void* src, size_t n);

uint32_t ipv4_fragment_packet(
        rte_mbuf* pkt_in, rte_mbuf** pkts_out,
        uint16_t nb_pkts_out, uint16_t mtu_size,
        struct rte_mempool* pool_direct,
        struct rte_mempool* pool_indirect) noexcept;
bool ipv4_frag_pkt_is_fragmented(const void* iph);
struct rte_mbuf* ipv4_frag_reassemble_packet(
        struct rte_ip_frag_tbl*        tbl,
        struct rte_ip_frag_death_row*  dr,
        struct rte_mbuf*               mb,
        uint64_t                       tms,
        struct ipv4_hdr*               iph
        );

void srand(uint64_t seedval);
uint64_t rand(void);

uint64_t get_tsc_cycles();
uint64_t get_tsc_hz();
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

template <class T>
T mtod(struct rte_mbuf* msg)
{
    return rte_pktmbuf_mtod(msg, T);
}

template <class T>
T pktmbuf_mtod(struct rte_mbuf* msg)
{
    return rte_pktmbuf_mtod(msg, T);
}
template <class T>
T pktmbuf_mtod_offset(struct rte_mbuf* msg, ssize_t offset)
{
    return rte_pktmbuf_mtod_offset(msg, T, offset);
}




} /* namespace rte */


class rte_mbuf_allocator {
    public:
        void deallocate(struct rte_mbuf* ptr) { rte::pktmbuf_free(ptr); }
};




