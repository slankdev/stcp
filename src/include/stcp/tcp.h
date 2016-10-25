

#pragma once


#include <stcp/config.h>
#include <stcp/socket.h>
#include <stcp/dpdk.h>

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
    STCP_TCP_ST_CLOSED      = 0,
    STCP_TCP_ST_LISTEN      = 1,
    STCP_TCP_ST_SYN_SENT    = 2,
    STCP_TCP_ST_SYN_RCVD    = 3,
    STCP_TCP_ST_ESTABLISHED = 4,
    STCP_TCP_ST_FIN_WAIT_1  = 5,
    STCP_TCP_ST_FIN_WAIT_2  = 6,
    STCP_TCP_ST_CLOSE_WAIT  = 7,
    STCP_TCP_ST_CLOSING     = 8,
    STCP_TCP_ST_LAST_ACK    = 9,
    STCP_TCP_ST_TIME_WAIT   = 10,
};



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
};





class stcp_tcp_sockdata {
};



struct tcp_stream_info {
    uint16_t my_port;   /* store as NetworkByteOrder */
    uint16_t pair_port; /* store as NetworkByteOrder */
    uint32_t seq_num;   /* store as NetworkByteOrder */
    uint32_t ack_num;   /* store as NetworkByteOrder */
};




class stcp_tcp_sock {
    friend class tcp_module;
    tcp_socket_state state;
    uint16_t port;

public:
    stcp_tcp_sock() : state(STCP_TCP_ST_CLOSED), port(0) {}
    ~stcp_tcp_sock(); // TODO IMPLE
    bool operator==(const stcp_tcp_sock& rhs) const { return port==rhs.port; }
    bool operator!=(const stcp_tcp_sock& rhs) const { return !(*this==rhs); }
    uint16_t get_port() const { return port; }

    static stcp_tcp_sock& socket();
    void close();
};




class tcp_module {
    friend class stcp_tcp_sock;
private:
    static size_t mss;
    size_t rx_cnt;
    size_t tx_cnt;
    std::vector<stcp_tcp_sock> socks;
    stcp_tcp_sock& socket();
    void close_socket(stcp_tcp_sock& s);

public:
    tcp_module() : rx_cnt(0), tx_cnt(0) {}
    void rx_push(mbuf* msg, stcp_sockaddr_in* src);
    void send_RSTACK(mbuf* msg, stcp_sockaddr_in* src, tcp_stream_info* info);
    // void tx_push(mbuf* msg, const stcp_sockaddr_in* dst, uint16_t srcp);
    void proc();
    void print_stat() const;
};





} /* namespace slank */
