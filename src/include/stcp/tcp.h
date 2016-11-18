

#pragma once


#include <stcp/config.h>
#include <stcp/socket.h>
#include <stcp/dpdk.h>
#include <stcp/stcp.h>

#include <vector>
#include <queue>




namespace slank {


enum tcp_flags {
    STCP_TCP_FLAG_FIN  = 0x01<<0, /* 00000001 */
    STCP_TCP_FLAG_SYN  = 0x01<<1, /* 00000010 */
    STCP_TCP_FLAG_RST  = 0x01<<2, /* 00000100 */
    STCP_TCP_FLAG_PSH  = 0x01<<3, /* 00001000 */
    STCP_TCP_FLAG_ACK  = 0x01<<4, /* 00010000 */
    STCP_TCP_FLAG_URG  = 0x01<<5, /* 00100000 */
    STCP_TCP_FLAG_SACK = 0x01<<6, /* 01000000 */
    STCP_TCP_FLAG_WACK = 0x01<<7, /* 10000000 */
};

enum tcp_socket_state {
    STCP_TCPS_CLOSED      = 0,
    STCP_TCPS_LISTEN      = 1,
    STCP_TCPS_SYN_SENT    = 2,
    STCP_TCPS_SYN_RCVD    = 3,
    STCP_TCPS_ESTABLISHED = 4,
    STCP_TCPS_FIN_WAIT_1  = 5,
    STCP_TCPS_FIN_WAIT_2  = 6,
    STCP_TCPS_CLOSE_WAIT  = 7,
    STCP_TCPS_CLOSING     = 8,
    STCP_TCPS_LAST_ACK    = 9,
    STCP_TCPS_TIME_WAIT   = 10,
};



inline const char* tcp_socket_state2str(tcp_socket_state state)
{
    switch (state) {
        case STCP_TCPS_CLOSED:
            return "CLOSED";
        case STCP_TCPS_LISTEN:
            return "LISTEN";
        case STCP_TCPS_SYN_SENT:
            return "SYN_SENT";
        case STCP_TCPS_SYN_RCVD:
            return "SYN_RCVD";
        case STCP_TCPS_ESTABLISHED:
            return "ESTABLISHED";
        case STCP_TCPS_FIN_WAIT_1:
            return "FIN_WAIT_1";
        case STCP_TCPS_FIN_WAIT_2:
            return "FIN_WAIT_2";
        case STCP_TCPS_CLOSE_WAIT:
            return "CLOSE_WAIT";
        case STCP_TCPS_CLOSING:
            return "CLOSING";
        case STCP_TCPS_LAST_ACK:
            return "LAST_ACK";
        case STCP_TCPS_TIME_WAIT:
            return "TIME_WAIT";
        default:
            return "UNKNOWN";
    }
}


struct stcp_tcp_header {
	uint16_t sport;     /**< TCP source port.            */
	uint16_t dport;     /**< TCP destination port.       */
	uint32_t seq_num;   /**< TX data sequence number.    */
	uint32_t ack_num;   /**< RX data ack number.         */
	uint8_t  data_off;  /**< Data offset.                */
	uint8_t  tcp_flags; /**< TCP flags                   */
	uint16_t rx_win;    /**< RX flow control window.     */
	uint16_t cksum;     /**< TCP checksum.               */
	uint16_t tcp_urp;   /**< TCP urgent pointer, if any. */

    void print() const
    {
        printf("TCP header \n");
        printf("+ sport    : %u 0x%04x \n", rte::bswap16(sport), rte::bswap16(sport)   );
        printf("+ dport    : %u 0x%04x \n", rte::bswap16(dport), rte::bswap16(dport)   );
        printf("+ seq num  : %u 0x%08x \n", rte::bswap32(seq_num), rte::bswap32(seq_num) );
        printf("+ ack num  : %u 0x%08x \n", rte::bswap32(ack_num), rte::bswap32(ack_num) );
        printf("+ data off : 0x%02x \n", data_off              );
        printf("+ tcp flags: 0x%02x \n", tcp_flags             );
        printf("+ rx win   : 0x%04x \n", rte::bswap16(rx_win)  );
        printf("+ cksum    : 0x%04x \n", rte::bswap16(cksum )  );
    }
};


struct stcp_tcp_sockdata {
    mbuf* msg;
    stcp_sockaddr_in addr;
};



#if 0
enum tcp_op_number : uint8_t {
    TCP_OP_FIN = 0x00,
    TCP_OP_NOP = 0x01,
    TCP_OP_MSS = 0x02,
};
struct tcp_op_fin {
    uint8_t op_num;
};
struct tcp_op_nop {
    uint8_t op_num;
};
struct tcp_op_mss {
    uint8_t op_num;
    uint8_t op_len;
    uint16_t seg_siz;
};
#endif



/*
 * All of variables are stored HostByteOrder
 */
struct currend_seg {
    uint32_t seg_seq; /* segument's sequence number    */
    uint32_t seg_ack; /* segument's acknouledge number */
    uint32_t seg_len; /* segument's length             */
    uint16_t seg_wnd; /* segument's window size        */
    uint16_t seg_up ; /* segument's urgent pointer     */
    uint32_t srg_prc; /* segument's priority           */

    currend_seg(const stcp_tcp_header* th, const stcp_ip_header* ih) :
        seg_seq(rte::bswap32(th->seq_num)),
        seg_ack(rte::bswap32(th->ack_num)),
        seg_len(rte::bswap16(ih->total_length) - th->data_off/4),
        seg_wnd(rte::bswap16(th->rx_win )),
        seg_up (rte::bswap16(th->tcp_urp)),
        srg_prc(rte::bswap32(0)) {}

    void print()
    {
        printf("Current Segument \n");
        printf(" - seq : %u(0x%x) \n", seg_seq, seg_seq);
        printf(" - ack : %u(0x%x) \n", seg_ack, seg_ack);
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
};


class stcp_tcp_sock {
    friend class tcp_module;
private:
    bool accepted;
    bool dead;
    stcp_tcp_sock* head;
    queue_TS<stcp_tcp_sock*> wait_accept;
    queue_TS<mbuf*> rxq;
    queue_TS<mbuf*> txq;

    size_t num_connected;
    size_t max_connect;

private:
    tcp_socket_state state;
    uint16_t port; /* store as NetworkByteOrder*/
    uint16_t pair_port; /* store as NetworkByteOrder */

#if 1
    stcp_sockaddr_in addr;
    stcp_sockaddr_in pair;
#else
    stcp_ip_header pip; /* TODO This field must be inited when connectin is established*/
#endif

    /*
     * Variables for TCP connected sequence number
     * All of variables are stored as HostByteOrder
     */
    uint32_t snd_una; /* unconfirmed send                  */ //?
    uint32_t snd_nxt; /* next send                         */ //?
    uint16_t snd_win; /* send window size                  */
    uint16_t snd_up ; /* send urgent pointer               */
    uint32_t snd_wl1; /* used sequence num at last send    */
    uint32_t snd_wl2; /* used acknouledge num at last send */
    uint32_t iss    ; /* initial send sequence number      */
    uint32_t rcv_nxt; /* next receive                      */
    uint16_t rcv_wnd; /* receive window size               */
    uint16_t rcv_up ; /* receive urgent pointer            */
    uint32_t irs    ; /* initial reseive sequence number   */

private:
    void move_state_from_CLOSED(tcp_socket_state next_state);
    void move_state_from_LISTEN(tcp_socket_state next_state);
    void move_state_from_SYN_SENT(tcp_socket_state next_state);
    void move_state_from_SYN_RCVD(tcp_socket_state next_state);
    void move_state_from_ESTABLISHED(tcp_socket_state next_state);
    void move_state_from_FIN_WAIT_1(tcp_socket_state next_state);
    void move_state_from_FIN_WAIT_2(tcp_socket_state next_state);
    void move_state_from_CLOSE_WAIT(tcp_socket_state next_state);
    void move_state_from_CLOSING(tcp_socket_state next_state);
    void move_state_from_LAST_ACK(tcp_socket_state next_state);
    void move_state_from_TIME_WAIT(tcp_socket_state next_state);

    void do_RST(stcp_tcp_header* th);
    void move_state_DEBUG(tcp_socket_state next_state);

    void proc();
    void print_stat() const;
    void rx_push(mbuf* msg, stcp_sockaddr_in* src);

public:
    stcp_tcp_sock();
    ~stcp_tcp_sock();
    void move_state(tcp_socket_state next_state);
    tcp_socket_state get_state() const { return state; }

public: /* for Users Operation */

    void bind(const struct stcp_sockaddr_in* addr, size_t addrlen);
    void listen(size_t backlog);
    stcp_tcp_sock* accept(struct stcp_sockaddr_in* addr);
    mbuf* read();
#if 0
    void write(mbuf* msg);
#endif
};



class tcp_module {
    friend class core;
private:
    static size_t mss;
    size_t rx_cnt;
    size_t tx_cnt;
    std::vector<stcp_tcp_sock*> socks;

public:
    tcp_module() : rx_cnt(0), tx_cnt(0) {}
    void rx_push(mbuf* msg, stcp_sockaddr_in* src);
    void tx_push(mbuf* msg, const stcp_sockaddr_in* dst, uint16_t srcp);
    void send_RSTACK(mbuf* msg, stcp_sockaddr_in* src);

    void proc();
    void print_stat() const;

};




} /* namespace slank */
