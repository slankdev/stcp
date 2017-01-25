


#include <stdio.h>
#include "dpdk/system.h"
#include "thread/txrx.h"
#include "thread/worker.h"
#include "thread/rtc.h"
#include "thread/omake.h"
#include "thread/viewer.h"
#include "sush/sush_main.h"


/*
 * TODO: Erase below
 * impelement cpu class startly.
 */
thread_txrx_arg port0_arg;
thread_txrx_arg port1_arg;

enum thread_pattern {
    TXRXWK,
    TXRX_WK,
    TX_RX_WK,
    TXRX0_TXRX1_WK,
    TX_RXWK,
    RX_TXWK,
};
void configure(dpdk::System* sys, thread_pattern pattern)
{
    // sys->cpus[1].thrd = {thread_viewer, sys};
    switch (pattern) {
        case TXRXWK:
            sys->cpus[2].thrd = {thread_txrxwk_RTC, sys};
            break;

        case TXRX_WK:
            sys->cpus[2].thrd = {thread_txrx_AP   , sys};
            sys->cpus[3].thrd = {thread_wk        , sys};
            break;

        case TX_RX_WK:
            sys->cpus[2].thrd = {thread_tx_AP     , sys};
            sys->cpus[3].thrd = {thread_rx_AP     , sys};
            sys->cpus[4].thrd = {thread_wk        , sys};
            break;

        case TXRX0_TXRX1_WK:
            port0_arg = {sys, 0};
            port1_arg = {sys, 1};
            sys->cpus[2].thrd = {thread_txrx      , &port0_arg};
            sys->cpus[3].thrd = {thread_txrx      , &port1_arg};
            sys->cpus[4].thrd = {thread_wk        , sys};
            break;

        case TX_RXWK:
            sys->cpus[2].thrd = {thread_tx_AP     , sys};
            sys->cpus[3].thrd = {thread_rxwk_AP   , sys};
            break;

        case RX_TXWK:
            sys->cpus[2].thrd = {thread_rx_AP     , sys};
            sys->cpus[3].thrd = {thread_txwk_AP   , sys};
            break;

        default:
            throw slankdev::exception("FAAAAAAA!!!!!");
            break;
    }
}

int main(int argc, char** argv)
{
    dpdk::System::rx_ring_size   = 128;
    dpdk::System::tx_ring_size   = 512;
    dpdk::System::port_bulk_size = 32;

    dpdk::System sys(argc, argv);
    if (sys.ports.size()%2 != 0) return -1;

    configure(&sys, TXRXWK);
    sys.cpus[1].thrd = {thread_sush, &sys};

    sys.launch();
}

