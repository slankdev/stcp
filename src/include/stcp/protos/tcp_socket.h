

#pragma once

#include <stcp/config.h>
#include <stcp/socket.h>
#include <stcp/dataplane.h>
#include <stcp/stcp.h>
#include <stcp/util.h>
#include <stcp/protos/tcp_var.h>
#include <stcp/protos/tcp.h>
#include <vector>




namespace slank {





class stcp_tcp_sock {
    friend class tcp_module;
    friend class core;
public:
    /*
     * for polling infos
     */
    bool readable()   { return !rxq.empty(); }
    bool acceptable() { return wait_accept_count > 0; }
    bool sockdead()   { return sock_state==SOCKS_UNUSE; }

private:
    stcp_tcp_sock* parent;

    queue_TS<mbuf*> rxq;
    queue_TS<mbuf*> txq;

    size_t wait_accept_count;
    size_t max_connect;

private:
    socketstate sock_state;
    tcpstate    tcp_state;
    uint16_t port;      /* NetworkByteOrder */
    uint16_t pair_port; /* NetworkByteOrder */
    stcp_sockaddr_in addr;
    stcp_sockaddr_in pair;
    tcp_stream_info si;

private:
    void proc();
    void print_stat(size_t rootx, size_t rooty) const;
    void rx_push(mbuf* msg, stcp_sockaddr_in* src);

public:
    void init();
    void term();
    stcp_tcp_sock();
    ~stcp_tcp_sock();
    void move_state(tcpstate next_state);
    tcpstate get_state() const { return tcp_state; }

public:
    /*
     * for Users Operation
     */
    void bind(const struct stcp_sockaddr_in* addr, size_t addrlen);
    void listen(size_t backlog);
    stcp_tcp_sock* accept(struct stcp_sockaddr_in* addr);
    mbuf* read();
    void write(mbuf* msg);

private:
    void move_state_from_CLOSED(tcpstate next_state);
    void move_state_from_LISTEN(tcpstate next_state);
    void move_state_from_SYN_SENT(tcpstate next_state);
    void move_state_from_SYN_RCVD(tcpstate next_state);
    void move_state_from_ESTABLISHED(tcpstate next_state);
    void move_state_from_FIN_WAIT_1(tcpstate next_state);
    void move_state_from_FIN_WAIT_2(tcpstate next_state);
    void move_state_from_CLOSE_WAIT(tcpstate next_state);
    void move_state_from_CLOSING(tcpstate next_state);
    void move_state_from_LAST_ACK(tcpstate next_state);
    void move_state_from_TIME_WAIT(tcpstate next_state);

private:
    /*
     * called by rx_push()
     */
    void rx_push_CLOSED(mbuf* msg, stcp_sockaddr_in* src);
    void rx_push_LISTEN(mbuf* msg, stcp_sockaddr_in* src);
    void rx_push_SYN_SEND(mbuf* msg, stcp_sockaddr_in* src);
    void rx_push_ELSESTATE(mbuf* msg, stcp_sockaddr_in* src);

    bool rx_push_ES_seqchk(mbuf* msg, stcp_sockaddr_in* src);
    bool rx_push_ES_rstchk(mbuf* msg, stcp_sockaddr_in* src);
    bool rx_push_ES_synchk(mbuf* msg, stcp_sockaddr_in* src);
    bool rx_push_ES_ackchk(mbuf* msg, stcp_sockaddr_in* src);
    bool rx_push_ES_textseg(mbuf* msg, stcp_sockaddr_in* src);
    bool rx_push_ES_finchk(mbuf* msg, stcp_sockaddr_in* src);
#if 0
    bool rx_push_ES_secprcchk(mbuf* msg, stcp_sockaddr_in* src);
    bool rx_push_ES_urgchk(mbuf* msg, stcp_sockaddr_in* src);
#endif
};




} /* namespace slank */


