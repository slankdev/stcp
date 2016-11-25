




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
    /*
     * 2: RST Check
     */
    /*
     * 3: Securty and Priority Check
     */
    /*
     * 4: SYN Check
     */
    /*
     * 5: (!SYN && !RST) Pattern
     */
}
void rx_push_ELSESTATE(mbuf* msg, sockaddr_in src, iph* ih, tcph* th)
{
    /*
     * 1: Sequence Number Check
     */
    /*
     * 2: RST Check
     */
    /*
     * 3: Securty and Priority Check
     */
    /*
     * 4: SYN Check
     */
    /*
     * 5: ACK Check
     */
    /*
     * 6: URG Check
     */
    /*
     * 4: Text Segment Control
     */
    /*
     * 6: FIN Check
     */

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

