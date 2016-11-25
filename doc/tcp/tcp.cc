




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

}
void rx_push_LISTEN(mbuf* msg, sockaddr_in src, iph* ih, tcph* th)
{
    /*
     * 1: RST Check
     */
    /*
     * 2: ACK Check
     */
    /*
     * 3: SYN Check
     */
    /*
     * 4: Else Text Control
     */
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

