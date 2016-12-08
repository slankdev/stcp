
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
#include <stcp/config.h>



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


inline size_t pktmbuf_data_len(const rte_mbuf* m)
{
    return m->data_len;
}

inline size_t pktmbuf_pkt_len(const rte_mbuf* m)
{
    return m->pkt_len;
}

inline void eth_dev_init(int argc, char** argv)
{
    int ret = rte_eal_init(argc, argv);
    if (ret < 0) {
        throw rte::exception("rte_eal_init");
    }
}

inline void eth_dev_configure(uint8_t port_id, uint16_t nb_rx_queue,
        uint16_t nb_tx_queue, const struct rte_eth_conf* eth_conf)
{
    int ret = rte_eth_dev_configure(port_id, nb_rx_queue, nb_tx_queue, eth_conf);
    if (ret != 0) {
        throw rte::exception("eth_eth_configure");
    }
}

inline void eth_rx_queue_setup(uint8_t port_id, uint16_t rx_queue_id,
        uint16_t nb_rx_desc, uint32_t socket_id,
        const struct rte_eth_rxconf* rx_conf, struct rte_mempool* mbuf_pool)
{
    int ret = rte_eth_rx_queue_setup(port_id, rx_queue_id,
            nb_rx_desc, socket_id, rx_conf, mbuf_pool);
    if (ret < 0) {
        throw rte::exception("rx_queue_setup");
    }
}

inline void eth_tx_queue_setup(uint8_t port_id, uint16_t tx_queue_id,
        uint16_t nb_tx_desc, uint32_t socket_id,
        const struct rte_eth_txconf* tx_conf)
{
    int ret = rte_eth_tx_queue_setup(port_id, tx_queue_id,
            nb_tx_desc, socket_id, tx_conf);
    if (ret < 0) {
        throw rte::exception("tx_queue_setup");
    }
}

inline void eth_dev_start(uint8_t port_id)
{
    int ret = rte_eth_dev_start(port_id);
    if (ret < 0) {
        throw rte::exception("rte_eth_dev_start");
    }
}

inline void eth_promiscuous_enable(uint8_t port_id)
{
    rte_eth_promiscuous_enable(port_id);
}

inline void eth_promiscuous_disable(uint8_t port_id)
{
    rte_eth_promiscuous_disable(port_id);
}

inline int eth_promiscuous_get(uint8_t port_id)
{
    return rte_eth_promiscuous_get(port_id);
}

inline struct rte_mempool* pktmbuf_pool_create(const char* name, uint32_t n,
        uint32_t cache_size, uint16_t priv_size,
        uint16_t data_room_size, int socket_id)
{
    rte_mempool* mbuf_pool = rte_pktmbuf_pool_create(
            name, n, cache_size, priv_size, data_room_size, socket_id);
    if (mbuf_pool == NULL)
        rte_exit(EXIT_FAILURE, "pktmbuf_pool_create()");
    return mbuf_pool;
}

inline size_t eth_dev_count()
{
    return rte_eth_dev_count();
}

inline size_t lcore_count()
{
    return rte_lcore_count();
}

inline void eth_macaddr_get(uint8_t port_id, struct ether_addr* mac_addr)
{
    rte_eth_macaddr_get(port_id, mac_addr);
}

inline uint16_t eth_rx_burst(uint8_t port_id, uint16_t queue_id,
        rte_mbuf** rx_pkts, const uint16_t nb_pkts)
{
    static uint64_t cnt = 1;
    uint16_t num_rx = rte_eth_rx_burst(port_id, queue_id, rx_pkts, nb_pkts);
    for (uint16_t i=0; i<num_rx; i++) {
        cnt++;
    }
    return num_rx;
}

inline uint16_t eth_tx_burst(uint8_t port_id, uint16_t queue_id,
        rte_mbuf** tx_pkts, uint16_t nb_pkts)
{
    static uint64_t cnt = 1;
    for (uint16_t i=0; i<nb_pkts; i++) {
        cnt++;
    }
    uint16_t num_tx = rte_eth_tx_burst(port_id, queue_id, tx_pkts, nb_pkts);
    return num_tx;
}

inline void pktmbuf_free(rte_mbuf* m)
{
    rte_pktmbuf_free(m);
}

inline uint32_t socket_id()
{
    return rte_socket_id();
}

inline void pktmbuf_dump(FILE* f,const rte_mbuf* m, unsigned dump_len)
{
    rte_pktmbuf_dump(f, m, dump_len);
    fprintf(f, "\n\n");
}

inline int eth_dev_socket_id(uint8_t port_id)
{
    return rte_eth_dev_socket_id(port_id);
}

inline rte_mbuf* pktmbuf_alloc(struct rte_mempool* mp)
{
    rte_mbuf* buf = rte_pktmbuf_alloc(mp);
    if (buf == nullptr) {
        throw rte::exception("rte_pktmbuf_alloc");
    }
    return buf;
}

inline rte_mbuf* pktmbuf_clone(rte_mbuf* md, struct rte_mempool* mp)
{
    rte_mbuf* buf = rte_pktmbuf_clone(md, mp);
    if (buf == nullptr) {
        throw rte::exception("rte_pktmbuf_clone");
    }
    return buf;
}

inline uint16_t pktmbuf_headroom(const rte_mbuf* m)
{
    return rte_pktmbuf_headroom(m);
}
inline uint16_t pktmbuf_tailroom(const rte_mbuf* m)
{
    return rte_pktmbuf_tailroom(m);
}
inline void pktmbuf_prepend(rte_mbuf* m, uint16_t len)
{
    char* ret = rte_pktmbuf_prepend(m, len);
    if (ret == nullptr) {
        throw rte::exception("rte_pktmbuf_prepend");
    }
}
inline void pktmbuf_append(rte_mbuf* m, uint16_t len)
{
    char* ret = rte_pktmbuf_append(m, len);
    if (ret == nullptr) {
        throw rte::exception("rte_pktmbuf_append");
    }
}
inline void pktmbuf_adj(rte_mbuf* m, uint16_t len)
{
    char* ret = rte_pktmbuf_adj(m, len);
    if (ret == nullptr) {
        throw rte::exception("rte_pktmbuf_adj");
    }
}
inline void pktmbuf_trim(rte_mbuf* m, uint16_t len)
{
    int ret = rte_pktmbuf_trim(m, len);
    if (ret == -1) {
        throw rte::exception("rte_pktmbuf_trim");
    }
}


inline size_t raw_cksum(const void* buf, size_t len)
{
    return rte_raw_cksum(buf, len);
}

inline uint16_t bswap16(uint16_t x)
{
    return rte_bswap16(x);
}
inline uint32_t bswap32(uint32_t x)
{
    return rte_bswap32(x);
}
inline uint64_t bswap64(uint64_t x)
{
    return rte_bswap64(x);
}

inline uint32_t ipv4_fragment_packet(rte_mbuf* pkt_in, rte_mbuf** pkts_out,
                    uint16_t nb_pkts_out,
                    uint16_t mtu_size, struct rte_mempool* pool_direct,
                    struct rte_mempool* pool_indirect) noexcept
{
    int32_t res = rte_ipv4_fragment_packet(pkt_in, pkts_out,
            nb_pkts_out, mtu_size, pool_direct, pool_indirect);
    if (res < 0) {
        return 1;
    }
    return res;
}

inline bool pktmbuf_is_contiguous(const rte_mbuf* m)
{
    return rte_pktmbuf_is_contiguous(m) == 1;
}

inline void* malloc(const char* type, size_t size, unsigned align)
{
    return rte_malloc(type, size, align);
}
inline void free(void* ptr)
{
    rte_free(ptr);
}

inline void* memcpy(void* dst, const void* src, size_t n)
{
    return rte_memcpy(dst, src, n);
}


inline void prefetch0(const volatile void* p)
{
    rte_prefetch0(p);
}
inline void prefetch1(const volatile void* p)
{
    rte_prefetch1(p);
}
inline void prefetch2(const volatile void* p)
{
    rte_prefetch2(p);
}

inline bool ipv4_frag_pkt_is_fragmented(const void* iph)
{
    int ret = rte_ipv4_frag_pkt_is_fragmented(
            reinterpret_cast<const ipv4_hdr*>(iph));
    return ret==1 ? true : false;
}

inline void srand(uint64_t seedval)
{
    rte_srand(seedval);
}

inline uint64_t rand(void)
{
    return rte_rand();
}

inline uint64_t get_tsc_cycles()
{
    return rte_get_tsc_cycles();
}
inline uint64_t get_tsc_hz()
{
    return rte_get_tsc_hz();
}
inline void delay_us(uint32_t us)
{
    return rte_delay_us(us);
}
inline void delay_ms(uint32_t ms)
{
    return rte_delay_ms(ms);
}

inline int eal_remote_launch(lcore_function_t *f, void *arg, unsigned slave_id)
{
    return rte_eal_remote_launch(f, arg, slave_id);
}


inline struct rte_mbuf* ipv4_frag_reassemble_packet(
        struct rte_ip_frag_tbl*        tbl,
        struct rte_ip_frag_death_row*  dr,
        struct rte_mbuf*               mb,
        uint64_t                       tms,
        struct ipv4_hdr*               iph    )
{
    return rte_ipv4_frag_reassemble_packet(tbl, dr, mb, tms, iph);
}



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





