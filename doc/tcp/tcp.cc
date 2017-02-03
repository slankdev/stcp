




enum State {
    TCPS_CLOSED     ,
    TCPS_LISTEN     ,
    TCPS_SYN_SENT   ,
    TCPS_SYN_RCVD   ,
    TCPS_ESTABLISHED,
    TCPS_FIN_WAIT_1 ,
    TCPS_FIN_WAIT_2 ,
    TCPS_CLOSE_WAIT ,
    TCPS_CLOSING    ,
    TCPS_LAST_ACK   ,
    TCPS_TIME_WAIT  ,
};
State state;

void rx_push_CLOSED(mbuf* msg, sockaddr_in src, iph* ih, tcph* th)
{
    if (HAVE(th, TCPF_RST)) {
        rte::pktmbuf_free(msg);
    } else {
        if HAVE(th, TCPF_ACK) {
            swap_port(th);
            th->ack_num = th->seq_num + datalen(th, ih);
            th->seq_num = 0;
            th->tcp_flags    = TCPF_RST|TCPF_ACK;
        } else {
            swap_port(th);
            th->seq_num = th->ack_num;
            th->tcp_flags    = TCPF_RST;
        }
        mbuf_pull(msg, iphlen);
        core::ip.tx_push(msg, src, 0x06);
    }
    return;
}
void rx_push_LISTEN(mbuf* msg, sockaddr_in src, iph* ih, tcph* th)
{
    /*
     * 1: TCPF_RST Check
     */
    if (HAVE(th, TCPF_RST)) {
        rte::pktmbuf_free(msg);
        return;
    }

    /*
     * 2: TCPF_ACK Check
     */
    if (HAVE(th, TCPF_ACK)) {
#if 0
        // in RFC
        swap_port(th);
        th->seq_num = th->ack_num;
        th->tcp_flags    = TCPF_RST;
        mbuf_pull(msg, iphlen);
        core::ip.tx_push(msg);
#else
        rte::pktmbuf_free(msg);
        return;
#endif
    }

    /*
     * 3: TCPF_SYN Check
     */
    if (HAVE(th, TCPF_SYN)) {
        if (!security_check(th)) {
            swap_port(th);
            th->seq_num = th->ack_num;
            th->tcp_flags    = TCPF_RST;
            mbuf_pull(msg, iphlen);
            core::ip.tx_push(msg, src, 0x06);
            return;
        }

        /*
         * Priority Check
         */
        if (prc(th) > this->prc) {
            if (allow_update_priority) {
                thi->prc = prc(th);
            } else {
                swap_port(th);
                th->seq_num = th->ack_num;
                th->flg = TCPF_RST;
                mbuf_pull(msg, iphlen);
                core::ip.tx_push(msg, src, 0x06);
                return;
            }
        }

        rcv_nxt = th->seq_num + 1;
        irs     = th->seq_num;
        iss     = rand();

        th->seq_num = iss;
        th->ack_num = rcv_nxt;
        th->tcp_flags    = TCPF_SYN|TCPF_ACK;

        mbuf_pull(msg, iphlen);
        core::ip.tx_push(msg, src, 0x06);

        snd_nxt = iss + 1;
        snd_una = iss;
        move_state(TCPF_SYN_RCVD);
        return;

    }

    /*
     * 4: Else Text Control
     */
    rte::pktmbuf_free(msg);
    throw exception("OKASHII");


}
void rx_push_SYN_SEND(mbuf* msg, sockaddr_in src, iph* ih, tcph* th)
{
    /*
     * 1: TCPF_ACK Check
     */
    if (HAVE(th, TCPF_ACK)) {
        if (th->ack_num <= iss || th->ack_num > snd_nxt) {
            if (HAVE(th, TCPF_RST)) {
                swap_port(th);
                th->seq_num = th->ack_num;
                th->tcp_flags    = TCPF_RST;

                mbuf_pull(msg, iphlen);
                core::ip.tx_push(msg, src, 0x06);
            else {
                rte::pktmbuf_free(msg);
            }
            printf("SLANKDEVSLANKDEV error: connection reset\n");
            move_state(CLOSED);
            return;
            }
        }
    }

    /*
     * 2: TCPF_RST Check
     */
    if (HAVE(th, TCPF_RST)) {
        rte::pktmbuf_free(msg);
        return;
    }

    /*
     * 3: Securty and Priority Check
     * TODO not implement yet
     */

    /*
     * 4: TCPF_SYN Check
     */
    assert(!HAVE(th, TCPF_ACK) && !HAVE(th, TCPF_RST));

    if (HAVE(th, TCPF_SYN)) {
        rcv_nxt = th->seq_num + 1;
        irs = th->seq_num;

        if (HAVE(th, TCPF_ACK)) {
            snd_una = th->ack_num;
        }

        if (snd_una > iss) {
            move_state(ESTABLISHED);
            swap_port(th);
            th->seq_num = snd_nxt;
            th->ack_num = rcv_nxt;
            th->tcp_flags = TCPF_ACK;
        } else {
            move_state(TCPF_SYN_RCVD);
            swap_port(th);
            th->seq_num = iss;
            th->ack_num = rcv_nxt;
            th->tcp_flags = TCPF_SYNTCPF_ACK;
        }
        mbuf_pull(msg, iphlen);
        core::ip.tx_push(msg, src, 0x06);
    }

    /*
     * 5: (!TCPF_SYN && !TCPF_RST) Pattern
     */
    if (!HAVE(th, TCPF_SYN) && !HAVE(th, TCPF_RST)) {
        rte::pktmbuf_free(msg);
        return;
    }

    throw exception("OKASHII");
}
void rx_push_ELSESTATE(mbuf* msg, sockaddr_in src, iph* ih, tcph* th)
{
    /*
     * 1: Sequence Number Check
     */
    switch (state) {

        case TCPS_SYN_RCVD:
        case TCPS_ESTABLISHED:
        case TCPS_FIN_WAIT_1:
        case TCPS_FIN_WAIT_2:
        case TCPS_CLOSE_WAIT:
        case TCPS_CLOSING:
        case TCPS_LAST_ACK:
        case TCPS_TIME_WAIT:
        {
            bool pass = false;
            if (datalen(th, ih) == 0) {
                if (th->win == 0) {
                    if (th->seq_num == rcv_nxt)
                        pass = true;
                } else { /* win > 0 */
                    if (rcv_nxt<=th->seq_num && th->seq_num<=rcv_nxt+rcv_win)
                        pass = true;
                }
            } else { /* datalen > 0 */
                if (th->win > 0) {
                    if ((rcv_nxt <= th->seq_num && th->seq_num < rcv_nxt+rcv_win)
                        || (rcv_nxt <= th->seq_num + datalen(th, ih) - 1 < rcv_nxt + rcv_win)) {
                        pass = true;
                    }
                }
            }

            if (!pass) {
                rte::pktmbuf_free(msg);
                return;
            }

            if (HAVE(th, TCPF_RST)) {
                rte::pktmbuf_free(msg);
                return;
            }

            swap_port(th);
            th->seq_num = snd_nxt;
            th->ack_num = rcv_nxt;
            th->tcp_flags = TCPF_ACK;

            mbuf_pull(msg, iphlen);
            core::ip.tx_push(msg, src, 0x06);
            break;
        }

        case TCPS_CLOSED:
        case TCPS_LISTEN:
        case TCPS_SYN_SENT:
            throw exception("OKASHII");
        default:
            throw exception("OKASHII: unknown state");
    }

    /*
     * 2: TCPF_RST Check
     */
    switch (state) {
        case TCPS_SYN_RCVD:
        {
            if (HAVE(th, TCPF_RST)) {
                printf("SLANKDEVSLANKDEV conection reset\n");
                rte::pktmbuf_free(msg);
                move_state(CLOSED);
                return;
            }
            break;
        }

        case TCPS_ESTABLISHED:
        case TCPS_FIN_WAIT_1:
        case TCPS_FIN_WAIT_2:
        case TCPS_CLOSE_WAIT:
        {
            if (HAVE(th, TCPF_RST)) {
                printf("SLANKDEVSLANKDEV conection reset\n");
                rte::pktmbuf_free(msg);
                move_state(CLOSED);
                return;
            }
            break;
        }

        case TCPS_CLOSING:
        case TCPS_LAST_ACK:
        case TCPS_TIME_WAIT:
        {
            if (HAVE(th, TCPF_RST)) {
                rte::pktmbuf_free(msg);
                move_state(CLOSED);
                return;
            }
            break;
        }

        case TCPS_CLOSED:
        case TCPS_LISTEN:
        case TCPS_SYN_SENT:
            throw exception("OKASHII");
        default:
            throw exception("OKASHII: unknown state");
    }

    /*
     * 3: Securty and Priority Check
     * TODO: not implement yet
     */

    /*
     * 4: TCPF_SYN Check
     */
    switch (state) {
        case TCPS_SYN_RCVD:
        case TCPS_ESTABLISHED:
        case TCPS_FIN_WAIT_1:
        case TCPS_FIN_WAIT_2:
        case TCPS_CLOSE_WAIT:
        case TCPS_CLOSING:
        case TCPS_LAST_ACK:
        case TCPS_TIME_WAIT:
        {
            if (HAVE(th, TCPF_SYN)) {
                printf("SLANKDEVSLANKDEV conection reset\n");
                rte::pktmbuf_free(msg);
                move_state(CLOSED);
                return;
            }
            break;
        }

        case TCPS_CLOSED:
        case TCPS_LISTEN:
        case TCPS_SYN_SENT:
            throw exception("OKASHII");
        default:
            throw exception("OKASHII: unknown state");
    }

    /*
     * 5: TCPF_ACK Check
     */
    if (HAVE(th, TCPF_ACK)) {
        switch (state) {
            case TCPS_SYN_RCVD:
            {
                if (snd_una <= th->ack_num && th->ack_num <= snd_nxt) {
                    move_state(ESTABLISHED);
                } else {
                    swap_port(th);
                    th->seq_num = th->ack_num;
                    th->tcp_flags = TCPF_RST;

                    mbuf_pull(msg, iphlen);
                    core::ip.tx_push(msg, src, 0x06);
                    return;
                }
                break;
            }

            case TCPS_ESTABLISHED:
            case TCPS_CLOSE_WAIT:
            case TCPS_CLOSING:
            {
                if (snd_una < th->ack_num && th->ack_num <= snd_nxt) {
                    snd_una = th->ack_num;
                }

                if (th->ack_num < snd_una) {
                    swap_port(th);
                    th->seq_num = th->ack_num;
                    th->tcp_flags = TCPF_RST;

                    mbuf_pull(msg, iphlen);
                    core::ip.tx_push(msg, src, 0x06);
                    return;
                }

                if (snd_una < th->ack_num && th->ack_num <= snd_nxt) {
                    if ((snd_wl1 < th->seq_num)
                            || (snd_wl1 == th->seq_num)
                            || (snd_wl2 == th->ack_num)) {
                        snd_win = th->win;
                        snd_wl1 = th->seq_num;
                        snd_wl2 = th->ack_num;
                    }
                }

                if (state == CLOSING) {
                    if (snd_nxt <= th->ack_num) {
                        move_state(TIME_WAIT);
                    }
                    rte::pktmbuf_free(msg);
                }
                break;
            }

            case TCPS_FIN_WAIT_1:
            {
                move_state(TCPF_FIN_WAIT_2);
                break;
            }
            case TCPS_FIN_WAIT_2:
            {
                printf("OK\n");
                break;
            }
            case TCPS_LAST_ACK:
            {
                if (snd_nxt <= th->ack_num) {
                    move_state(CLOSED);
                    return;
                }
                break;
            }
            case TCPS_TIME_WAIT:
            {
                throw exception("TODO: NOT IMPEL YET");
                break;
            }

            case TCPS_CLOSED:
            case TCPS_LISTEN:
            case TCPS_SYN_SENT:
                throw exception("OKASHII");
            default:
                throw exception("OKASHII: unknown state");
        }

    } else {
        rte::pktmbuf_free(msg);
        return;
    }

    /*
     * 6: URG Check
     * TODO: not implement yet
     */
    /*
     * 4: Text Segment Control
     */
    switch (state) {
        case TCPS_ESTABLISHED:
        case TCPS_FIN_WAIT_1:
        case TCPS_FIN_WAIT_2:
        {
            rcv_nxt += datalen(th, ih);

            swap_port(th);
            th->seq_num = snd_nxt;
            th->ack_num = rcv_nxt;
            th->tcp_flags = TCPF_ACK;

            mbuf_pull(msg, iphlen);
            core::ip.tx_push(msg, src, 0x06);
            return;
            break;
        }

        case TCPS_CLOSE_WAIT:
        case TCPS_CLOSING:
        case TCPS_LAST_ACK:
        case TCPS_TIME_WAIT:

        case TCPS_CLOSED:
        case TCPS_LISTEN:
        case TCPS_SYN_SENT:
            throw exception("OKASHII");

        case TCPS_SYN_RCVD:
            throw exception("RFC MITEIGIIIII");

        default:
            throw exception("OKASHII: unknown state");
    }

    /*
     * 6: TCPF_FIN Check
     */
    if (HAVE(th, TCPF_FIN)) {
        printf("SLANKDEVSLANKDEV connection closing\n");
        switch (state) {
            case TCPS_CLOSED:
            case TCPS_LISTEN:
            case TCPS_SYN_SENT:
                rte::pktmbuf_free(msg);
                return;
                break;
            case TCPS_SYN_RCVD:
            case TCPS_ESTABLISHED:
                move_state(CLOSE_WAIT);
                break;
            case TCPS_FIN_WAIT_1:
                move_state(CLOSING);
                break;
            case TCPS_FIN_WAIT_2:
                move_state(TIME_WAIT);
                break;
            case TCPS_CLOSE_WAIT:
                break;
            case TCPS_CLOSING:
                break;
            case TCPS_LAST_ACK:
                break;
            case TCPS_TIME_WAIT:
                break;

            default:
                throw exception("OKASHII: unknown state");
        }
        return;
    }
}

void rx_push(mbuf* msg, sockaddr_in src)
{
    iph*  ih = mtod(msg);
    tcph* th = mtod_offset(msg);

    switch (state) {
        case TCPS_CLOSED:
            rx_push_CLOSED(msg, src, ih, th);
            break;
        case TCPS_LISTEN:
            rx_push_LISTEN(msg, src, ih, th);
            break;
        case TCPS_SYN_SENT:
            rx_push_SYN_SEND(msg, src, ih, th);
            break;
        case TCPS_SYN_RCVD:
        case TCPS_ESTABLISHED:
        case TCPS_FIN_WAIT_1:
        case TCPS_FIN_WAIT_2:
        case TCPS_CLOSE_WAIT:
        case TCPS_CLOSING:
        case TCPS_LAST_ACK:
        case TCPS_TIME_WAIT:
            rx_push_ELSESTATE(msg, src, ih, th);
            break;
        default:
            throw exception("OKASHII");
    }
}

