


#pragma once
#include <ssnlib_mempool.h>
#include <susanoo_log.h>


namespace dpdk {

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
    const size_t port_id;
    const size_t queue_id;
public:
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

        kernel_log(SYSTEM, "init ring %s ... done\n", name());
    }
    ~Ring() { if (!ring_) rte_ring_free(ring_); }

    virtual void burst_bulk() = 0;

#if 0
    /*
     * If ring is already full, this container
     * frees 10 element and re-enqueue.
     */
    void push(rte_mbuf* data)
    {
        if (!data) return;

        int ret = rte_ring_enqueue(ring_, data);
		if (ret < 0) {
			if (ret == -EDQUOT) {
                /*
                 * Quota exceeded.
                 * The objects have been enqueued,
                 * but the high water mark is exceeded.
                 */
			} else if (ret == -ENOBUFS) {
                /*
                 * Not enough room in the ring to enqueue;
                 * no object is enqueued.
                 */
                rte_mbuf* m;
                for (size_t i=0; i<10; i++) {
                    pop(&m);
                    rte_pktmbuf_free(m);
                }
                push(data);
			} else {
				throw slankdev::exception("rte_ring_enqueue: unknown");
			}
		}
    }
    /*
     * If ring is empty, *data = nullptr;
     */
    bool pop(rte_mbuf** data)
    {
        int ret = rte_ring_dequeue(ring_, reinterpret_cast<void**>(data));
        if (ret < 0) {
			if (ret == -ENOENT) {
                /*
                 * Not enough entries in the ring to dequeue,
                 * no object is dequeued.
                 */
                *data = nullptr;
			}
            return false;
		}
        return true;

    }
#endif

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
    bool   full()  const { return rte_ring_full(ring_)==1;  }
    const char* name() const { return (const char*)ring_->name; }
};


class Rxq : public Ring {
public:
    Rxq(const char* n, size_t count, uint16_t socket_id, size_t p, size_t q)
        : Ring(n, count, socket_id, p, q) {}
    void burst_bulk()
    {
        size_t bulk_size = 32;
        struct rte_mbuf* rx_pkts[bulk_size];
        uint16_t nb_rx = rte_eth_rx_burst(port_id, queue_id, rx_pkts, bulk_size);
        if (nb_rx == 0) return;
        push_bulk(rx_pkts, nb_rx);
    }
};


class Txq : public Ring {
public:
    Txq(const char* n, size_t count, uint16_t socket_id, size_t p, size_t q)
        : Ring(n, count, socket_id, p, q) {}
    void burst_bulk()
    {
        size_t bulk_size = 32;
        struct rte_mbuf* pkts[bulk_size];
        bool ret = pop_bulk(pkts, bulk_size);
        if (ret == true) {
            uint16_t nb_tx = rte_eth_tx_burst(port_id, queue_id, pkts, bulk_size);
            if (nb_tx != bulk_size) {
                rte_pktmbuf_free_bulk(&pkts[nb_tx], bulk_size-nb_tx);
            }
        }
    }
};




} /* namespace dpdk */
