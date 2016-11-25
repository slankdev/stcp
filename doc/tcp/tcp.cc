




enum State {
    CLOSED     ,
    LISTEN     ,
    SYN_SENT   ,
    SYN_RCVD   ,
    ESTABLISHED,
    FIN_WAIT_1 ,
    FIN_WAIT_2 ,
    CLOSE_WAIT ,
    CLOSING    ,
    LAST_ACK   ,
    TIME_WAIT  ,
};
State state;

void rx_push_CLOSED(mbuf* msg, sockaddr_in src, iph* ih, tcph* th)
{
    if (HAVE(th, RST)) {
        free(msg);
    } else {
        if HAVE(th, ACK) {
            swap_port(th);
            th->ack_num = th->seq_num + datalen(th, ih);
            th->seq_num = 0;
            th->flag    = RST|ACK;
        } else {
            swap_port(th);
            th->seq_num = th->ack_num;
            th->flag    = RST;
        }
        mbuf_pull(msg, iphlen);
        core::ip.tx_push(msg, src, 0x06);
    }
    return;
}
void rx_push_LISTEN(mbuf* msg, sockaddr_in src, iph* ih, tcph* th)
{
    /*
     * 1: RST Check
     */
    if (HAVE(th, RST)) {
        free(msg);
        return;
    }

    /*
     * 2: ACK Check
     */
    if (HAVE(th, ACK)) {
#if 0
        // in RFC
        swap_port(th);
        th->seq_num = th->ack_num;
        th->flag    = RST;
        mbuf_pull(msg, iphlen);
        core::ip.tx_push(msg);
#else
        free(msg);
        return;
#endif
    }

    /*
     * 3: SYN Check
     */
    if (HAVE(th, SYN)) {
        if (!security_check(th)) {
            swap_port(th);
            th->seq_num = th->ack_num;
            th->flag    = RST;
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
                th->flg = RST;
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
        th->flag    = SYN|ACK;

        mbuf_pull(msg, iphlen);
        core::ip.tx_push(msg, src, 0x06);

        snd_nxt = iss + 1;
        snd_una = iss;
        change_state(SYN_RCVD);
        return;

    }

    /*
     * 4: Else Text Control
     */
    free(msg);
    throw exception("OKASHII");


}
void rx_push_SYN_SEND(mbuf* msg, sockaddr_in src, iph* ih, tcph* th)
{
    /*
     * 1: ACK Check
     */
    if (HAVE(th, ACK)) {
        if (th->ack_num <= iss || th->ack_num > snd_nxt) {
            if (HAVE(th, RST)) {
                swap_port(th);
                th->seq_num = th->ack_num;
                th->flag    = RST;

                mbuf_pull(msg, iphlen);
                core::ip.tx_push(msg, src, 0x06);
            else {
                free(msg);
            }
            printf("SLANKDEVSLANKDEV error: connection reset\n");
            change_state(CLOSED);
            return;
            }
        }
    }

    /*
     * 2: RST Check
     */
    if (HAVE(th, RST)) {
        free(msg);
        return;
    }

    /*
     * 3: Securty and Priority Check
     * TODO not implement yet
     */

    /*
     * 4: SYN Check
     */
    assert(!HAVE(th, ACK) && !HAVE(th, RST));

    if (HAVE(th, SYN)) {
        rcv_nxt = th->seq_num + 1;
        irs = th->seq_num;

        if (HAVE(th, ACK)) {
            snd_una = th->ack_num;
        }

        if (snd_una > iss) {
            change_state(ESTABLISHED);
            swap_port(th);
            th->seq_num = snd_nxt;
            th->ack_num = rcv_nxt;
            th->flag = ACK;
        } else {
            change_state(SYN_RCVD);
            swap_port(th);
            th->seq_num = iss;
            th->ack_num = rcv_nxt;
            th->flag = SYNACK;
        }
        mbuf_pull(msg, iphlen);
        core::ip.tx_push(msg, src, 0x06);
    }

    /*
     * 5: (!SYN && !RST) Pattern
     */
    if (!HAVE(th, SYN) && !HAVE(th, RST)) {
        free(msg);
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

        case SYN_RCVD:
        case ESTABLISHED:
        case FIN_WAIT_1:
        case FIN_WAIT_2:
        case CLOSE_WAIT:
        case CLOSING:
        case LAST_ACK:
        case TIME_WAIT:
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
                free(msg);
                return;
            }

            if (HAVE(th, RST)) {
                free(msg);
                return;
            }

            swap_port(th);
            th->seq_num = snd_nxt;
            th->ack_num = rcv_nxt;
            th->flag = ACK;

            mbuf_pull(msg, iphlen);
            core::ip.tx_push(msg, src, 0x06);
            break;
        }

        case CLOSED:
        case LISTEN:
        case SYN_SENT:
            throw exception("OKASHII");
        default:
            throw exception("OKASHII: unknown state");
    }

    /*
     * 2: RST Check
     */
    switch (state) {
        case SYN_RCVD:
        {
            if (HAVE(th, RST)) {
                printf("SLANKDEVSLANKDEV conection reset\n");
                free(msg);
                change_state(CLOSED);
                return;
            }
            break;
        }

        case ESTABLISHED:
        case FIN_WAIT_1:
        case FIN_WAIT_2:
        case CLOSE_WAIT:
        {
            if (HAVE(th, RST)) {
                printf("SLANKDEVSLANKDEV conection reset\n");
                free(msg);
                change_state(CLOSED);
                return;
            }
            break;
        }

        case CLOSING:
        case LAST_ACK:
        case TIME_WAIT:
        {
            if (HAVE(th, RST)) {
                free(msg);
                change_state(CLOSED);
                return;
            }
            break;
        }

        case CLOSED:
        case LISTEN:
        case SYN_SENT:
            throw exception("OKASHII");
        default:
            throw exception("OKASHII: unknown state");
    }

    /*
     * 3: Securty and Priority Check
     * TODO: not implement yet
     */

    /*
     * 4: SYN Check
     */
    switch (state) {
        case SYN_RCVD:
        case ESTABLISHED:
        case FIN_WAIT_1:
        case FIN_WAIT_2:
        case CLOSE_WAIT:
        case CLOSING:
        case LAST_ACK:
        case TIME_WAIT:
        {
            if (HAVE(th, SYN)) {
                printf("SLANKDEVSLANKDEV conection reset\n");
                free(msg);
                change_state(CLOSED);
                return;
            }
            break;
        }

        case CLOSED:
        case LISTEN:
        case SYN_SENT:
            throw exception("OKASHII");
        default:
            throw exception("OKASHII: unknown state");
    }

    /*
     * 5: ACK Check
     */
    if (HAVE(th, ACK)) {
        switch (state) {
            case SYN_RCVD:
            {
                if (snd_una <= th->ack_num && th->ack_num <= snd_nxt) {
                    change_state(ESTABLISHED);
                } else {
                    swap_port(th);
                    th->seq_num = th->ack_num;
                    th->flag = RST;

                    mbuf_pull(msg, iphlen);
                    core::ip.tx_push(msg, src, 0x06);
                    return;
                }
                break;
            }

            case ESTABLISHED:
            case CLOSE_WAIT:
            case CLOSING:
            {
                if (snd_una < th->ack_num && th->ack_num <= snd_nxt) {
                    snd_una = th->ack_num;
                }

                if (th->ack_num < snd_una) {
                    swap_port(th);
                    th->seq_num = th->ack_num;
                    th->flag = RST;

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
                        change_state(TIME_WAIT);
                    }
                    free(msg);
                }
                break;
            }

            case FIN_WAIT_1:
            {
                change_state(FIN_WAIT_2);
                break;
            }
            case FIN_WAIT_2:
            {
                printf("OK\n");
                break;
            }
            case LAST_ACK:
            {
                if (snd_nxt <= th->ack_num) {
                    change_state(CLOSED);
                    return;
                }
                break;
            }
            case TIME_WAIT:
            {
                throw exception("TODO: NOT IMPEL YET");
                break;
            }

            case CLOSED:
            case LISTEN:
            case SYN_SENT:
                throw exception("OKASHII");
            default:
                throw exception("OKASHII: unknown state");
        }

    } else {
        free(msg);
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
        case ESTABLISHED:
        case FIN_WAIT_1:
        case FIN_WAIT_2:
        {
            rcv_nxt += datalen(th, ih);

            swap_port(th);
            th->seq_num = snd_nxt;
            th->ack_num = rcv_nxt;
            th->flag = ACK;

            mbuf_pull(msg, iphlen);
            core::ip.tx_push(msg, src, 0x06);
            return;
            break;
        }

        case CLOSE_WAIT:
        case CLOSING:
        case LAST_ACK:
        case TIME_WAIT:

        case CLOSED:
        case LISTEN:
        case SYN_SENT:
            throw exception("OKASHII");

        case SYN_RCVD:
            throw exception("RFC MITEIGIIIII");

        default:
            throw exception("OKASHII: unknown state");
    }

    /*
     * 6: FIN Check
     */
    if (HAVE(th, FIN)) {
        printf("SLANKDEVSLANKDEV connection closing\n");
        switch (state) {
            case CLOSED:
            case LISTEN:
            case SYN_SENT:
                free(msg);
                return;
                break;
            case SYN_RCVD:
            case ESTABLISHED:
                change_state(CLOSE_WAIT);
                break;
            case FIN_WAIT_1:
                change_state(CLOSING);
                break;
            case FIN_WAIT_2:
                change_state(TIME_WAIT);
                break;
            case CLOSE_WAIT:
                break;
            case CLOSING:
                break;
            case LAST_ACK:
                break;
            case TIME_WAIT:
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
        case CLOSED:
            rx_push_CLOSED(msg, src, ih, th);
            break;
        case LISTEN:
            rx_push_LISTEN(msg, src, ih, th);
            break;
        case SYN_SENT:
            rx_push_SYN_SEND(msg, src, ih, th);
            break;
        case SYN_RCVD:
        case ESTABLISHED:
        case FIN_WAIT_1:
        case FIN_WAIT_2:
        case CLOSE_WAIT:
        case CLOSING:
        case LAST_ACK:
        case TIME_WAIT:
            rx_push_ELSESTATE(msg, src, ih, th);
            break;
        default:
            throw exception("OKASHII");
    }
}

