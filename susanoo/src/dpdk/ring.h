


#pragma once
#include "mempool.h"
#include "log.h"


namespace dpdk {

static inline void __attribute__((always_inline))
rte_pktmbuf_free_bulk(struct rte_mbuf *m_list[], int16_t npkts)
{
	while (npkts--) {
		rte_pktmbuf_free(*m_list++);
    }
}



class Ring {
    struct rte_ring* ring_;
	size_t ring_depth;
public:
    Ring(const char* n, size_t count, uint16_t socket_id)
        : ring_depth(count)
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
    ~Ring()
    {
        if (!ring_)
            rte_ring_free(ring_);
    }

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


#if 0
    /*
     * If ring is empty, *data = nullptr;
     */
    void pop(rte_mbuf** data)
    {
        int ret = rte_ring_dequeue(ring_, reinterpret_cast<void**>(data));
        if (ret < 0) {
			if (ret == -ENOENT) {
                /*
                 * Not enough entries in the ring to dequeue,
                 * no object is dequeued.
                 */
                *data = nullptr;
			} else {
				throw slankdev::exception("rte_ring_dequeue: unknown");
			}
		}
    }
#endif

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


} /* namespace dpdk */
