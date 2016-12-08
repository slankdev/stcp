

#pragma once

#include <stcp/arch/dpdk/rte.h>
#include <queue>
#include <mutex>
#include <stdio.h>
#include <stdarg.h>
#include <stcp/ncurses.h>


namespace slank {

extern ncurses screen;

#define DEBUG

#ifdef DEBUG
#define DPRINT(...) \
  { \
      char str[10000]; \
      sprintf(str, __VA_ARGS__); \
      screen.debugprintw("%-15s:%4d: %s", __FILE__, __LINE__, str); \
  }
#else
#define DPRINT(...)
#endif

using eth_conf = struct rte_eth_conf;
using mbuf = struct rte_mbuf;
using mempool = struct rte_mempool;
using ip_frag_death_row = struct rte_ip_frag_death_row;
using ip_frag_tbl       = struct rte_ip_frag_tbl;

inline int stcp_printf(const char* format, ...)
{
    size_t rootx = screen.POS_STDO.x;
    size_t rooty = screen.POS_STDO.y;

    static int cur = 0;
    if (cur > 10) cur = 0;
    else cur++;

    char str[1000];
    va_list arg;
    va_start(arg, format);
    int ret = vsprintf(str, format, arg);
    va_end(arg);

    screen.mvprintw(rooty+cur, rootx, "%-15s %4s: %s", "STCP_PRINTF", "", str);
    return ret;
}


class pkt_queue {
    std::queue<mbuf*> queue;
public:
    void push(mbuf* msg)
    {
        queue.push(msg);
    }
    mbuf* pop()
    {
        mbuf* msg = queue.front();
        queue.pop();
        return msg;
    }
    size_t size() const
    {
        return queue.size();
    }
    bool empty() const
    {
        return queue.empty();
    }
};


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
