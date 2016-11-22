

#pragma once

#include <stcp/rte.h>
#include <stcp/exception.h>
#include <stcp/log.h>
#include <queue>
#include <mutex>
#include <stdio.h>


namespace slank {

#define DEBUG(...) \
    printf("%-10s:%4d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__)

using eth_conf = struct rte_eth_conf;
using mbuf = struct rte_mbuf;
using pkt_queue = std::queue<struct rte_mbuf*>;

template<class T>
class queue_TS {
    std::queue<T> queue;
    mutable std::mutex m;
    using auto_lock=std::lock_guard<std::mutex>;
public:
    void push(T msg)
    {
        auto_lock lg(m);
        queue.push(msg);
    }
    T pop()
    {
        auto_lock lg(m);
        T msg = queue.front();
        queue.pop();
        return msg;
    }
    size_t size() const
    {
        auto_lock lg(m);
        return queue.size();
    }
    bool empty() const
    {
        auto_lock lg(m);
        return queue.empty();
    }
};

} /* namespace */
