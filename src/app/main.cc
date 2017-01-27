

#include <stdio.h>
#include <susanoo.h>
#include "usr_thrds.h"
#include "usr_cmds.h"



int main(int argc, char** argv)
{
    System::nb_rx_rings    = 2;
    System::nb_tx_rings    = 2;
    System::rx_ring_size   = 128;
    System::tx_ring_size   = 512;
    System::port_bulk_size = 32;

    System sys(argc, argv);
    if (sys.ports.size()%2 != 0) return -1;

    sys.shell.add_cmd(new Cmd_ifconfig(&sys));
    sys.shell.add_cmd(new Cmd_test    (&sys));

#if 1
    ssnt_txrxwk txrxwk(&sys);
    sys.cpus[0].thrd = &sys.shell;
    sys.cpus[1].thrd = &txrxwk;
#else
    ssnt_rx rx(&sys);
    ssnt_tx tx(&sys);
    ssnt_wk wk(&sys);
    sys.cpus[0].thrd = &sys.shell;
    sys.cpus[1].thrd = &rx;
    sys.cpus[2].thrd = &tx;
    sys.cpus[3].thrd = &wk;
#endif

    sys.launch();
}

