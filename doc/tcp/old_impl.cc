
void stcp_tcp_sock::rx_push_LISTEN(mbuf* msg, stcp_sockaddr_in* src,
        stcp_ip_header* ih, stcp_tcp_header* th)
{
    if (th->tcp_flags == TCPF_SYN) {

        if (max_connect <= num_connected)
            throw exception("No such space to connect");

        /*
         * Recv SYN
         *
         * Tasks
         * + Create new socket.
         * + Init stream information.
         * + Craft SYNACK packet to reply.
         * + Ctrl Mbuf and send it.
         * + Update Stream information.
         */

        /*
         * Create new socket
         */
        stcp_tcp_sock* newsock = alloc_new_sock_connected(
                TCPS_SYN_RCVD, port, th->sport,
                rte::rand() % 0xffffffff, rte::bswap32(th->seq_num), this);
        num_connected ++;
        wait_accept.push(newsock);

        newsock->addr.sin_addr = ih->dst;
        newsock->pair.sin_addr = ih->src;
        // DEBUG("[%15p] connect request. alloc sock [%15p]\n", this, newsock);

        /*
         * Init stream information
         */
        newsock->si.snd_nxt_H(newsock->si.iss_H());
        newsock->si.snd_win_H(512); // TODO hardcode

        newsock->si.rcv_nxt_H(newsock->si.irs_H() + 1);
        newsock->si.rcv_win_N(th->rx_win);

        /*
         * craft SYNACK packet to reply.
         */
        swap_port(th);

        th->seq_num   = newsock->si.snd_nxt_N();
        th->ack_num   = newsock->si.rcv_nxt_N();

        th->rx_win    = newsock->si.snd_win_N();
        th->tcp_flags = TCPF_SYN | TCPF_ACK;
        th->cksum     = 0x0000;
        th->tcp_urp   = 0x0000; // TODO hardcode

        th->cksum = rte_ipv4_udptcp_cksum(
                reinterpret_cast<ipv4_hdr*>(ih), th);

        /*
         * Ctrl Mbuf and send it.
         * + pull mbuf
         * + send mbuf
         */
        mbuf_pull(msg, sizeof(stcp_ip_header));
        core::ip.tx_push(msg, src, STCP_IPPROTO_TCP);

        /*
         * Update stream information.
         */
        newsock->si.snd_nxt_inc_H(1);
        return ;
    }
}
