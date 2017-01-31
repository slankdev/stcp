


#pragma once
#include <ssnlib_log.h>
#include <ssnlib_mempool.h>


namespace ssnlib {

static inline void __attribute__((always_inline))
rte_pktmbuf_free_bulk(struct rte_mbuf *m_list[], int16_t npkts)
{
	while (npkts--) {
		rte_pktmbuf_free(*m_list++);
    }
}

class Ring {
protected:
    struct rte_ring* ring_;
	size_t ring_depth;
public:
    const size_t port_id;
    const size_t queue_id;
    Ring(const char* n, size_t count, uint16_t socket_id,
            size_t p, size_t q)
        : ring_depth(count), port_id(p), queue_id(q)
    {
        ring_ = rte_ring_create(n, count, socket_id, 0);
        if (!ring_) {
            char errstr[256];
            snprintf(errstr, sizeof(errstr),
                    "rte_ring_create(%s, %zd, %u)",
                    n, count, socket_id);
            throw slankdev::exception(errstr);
        }

        kernel_log(SYSTEM, "init ring %s ... done\n", ring_->name);
    }
    ~Ring() { if (!ring_) rte_ring_free(ring_); }
    void push_bulk(rte_mbuf** obj_table, size_t n)
    {
        int ret = rte_ring_enqueue_bulk(ring_, reinterpret_cast<void**>(obj_table), n);
        if (ret < 0) {
            if (ret == -EDQUOT ) {
                /*
                 * Quota exceeded.
                 * The objects have been enqueued,
                 * but the high water mark is exceeded.
                 */
            }
            else if (ret == -ENOBUFS) {
                /*
                 * Not enough room in the ring to enqueue;
                 * no object is enqueued.
                 */
                struct rte_mbuf* pkts[n];
                bool ret = pop_bulk(pkts, n);
                if (ret) rte_pktmbuf_free_bulk(pkts, n);
                push_bulk(obj_table, n);
            } else {
                throw slankdev::exception("rte_ring_enqueue_bulk: unknown");
            }
        }
    }
    bool pop_bulk(rte_mbuf** obj_table, size_t n)
    {
        int ret = rte_ring_dequeue_bulk(ring_, reinterpret_cast<void**>(obj_table), n);
        if (ret < 0) {
            if (ret == -ENOENT) {
                /*
                 * Not enough entries in the ring to dequeue,
                 * no object is dequeued.
                 */
            }
            return false;
        }
        return true;
    }
	size_t count() const { return rte_ring_count(ring_);    }
    size_t size()  const { return ring_depth;               }
    bool   empty() const { return rte_ring_empty(ring_)==1; }
};



class ssn_ring {
public:
    virtual void push_bulk(rte_mbuf** obj_table, size_t n) = 0;
    virtual bool pop_bulk(rte_mbuf** obj_table, size_t n)  = 0;
    virtual size_t count() const = 0;
    virtual size_t size() const  = 0;
    virtual bool empty() const   = 0;
    virtual void burst_bulk()    = 0;
};




template <class RING=Ring>
class Rxq : public ssn_ring {
    RING ring_impl;
public:

    Rxq(const char* n, size_t count, uint16_t socket_id, size_t p, size_t q)
        : ring_impl(n, count, socket_id, p, q) {}
    void push_bulk(rte_mbuf** obj_table, size_t n) { ring_impl.push_bulk(obj_table, n); }
    bool pop_bulk(rte_mbuf** obj_table, size_t n) { return ring_impl.pop_bulk(obj_table, n); }
    size_t count() const { return ring_impl.count(); }
    size_t size()  const { return ring_impl.size();  }
    bool   empty() const { return ring_impl.empty(); }

    void burst_bulk()
    {
        size_t bulk_size = 32;
        struct rte_mbuf* rx_pkts[bulk_size];
        uint16_t nb_rx = rte_eth_rx_burst(ring_impl.port_id, ring_impl.queue_id, rx_pkts, bulk_size);
        if (nb_rx == 0) return;
        ring_impl.push_bulk(rx_pkts, nb_rx);
    }
};


template <class RING=Ring>
class Txq : public ssn_ring {
    RING ring_impl;
public:
    Txq(const char* n, size_t count, uint16_t socket_id, size_t p, size_t q)
        : ring_impl(n, count, socket_id, p, q) {}

    void push_bulk(rte_mbuf** obj_table, size_t n) { ring_impl.push_bulk(obj_table, n); }
    bool pop_bulk(rte_mbuf** obj_table, size_t n) { return ring_impl.pop_bulk(obj_table, n); }
    size_t count() const  { return ring_impl.count(); }
    size_t size() const  { return ring_impl.size(); }
    bool empty() const  { return ring_impl.empty(); }

    void burst_bulk()
    {
        size_t bulk_size = 32;
        struct rte_mbuf* pkts[bulk_size];
        bool ret = ring_impl.pop_bulk(pkts, bulk_size);
        if (ret == true) {
            uint16_t nb_tx = rte_eth_tx_burst(ring_impl.port_id, ring_impl.queue_id, pkts, bulk_size);
            if (nb_tx != bulk_size) {
                rte_pktmbuf_free_bulk(&pkts[nb_tx], bulk_size-nb_tx);
            }
        }
    }
};




} /* namespace ssnlib */

