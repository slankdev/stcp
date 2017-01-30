

#include <stdio.h>
#include <ssnlib_sys.h>
#include <ssnlib_shell.h>
#include "usr_thrds.h"
#include "usr_cmds.h"



int main(int argc, char** argv)
{
    using namespace ssnlib;

    System::nb_rx_rings    = 2;
    System::nb_tx_rings    = 2;
    System::rx_ring_size   = 128;
    System::tx_ring_size   = 512;
    System::port_bulk_size = 32;

    System sys(argc, argv);
    if (sys.ports.size()%2 != 0) return -1;

    Shell shell(&sys);
    shell.add_cmd(new Cmd_test    (&sys, &shell));
    shell.add_cmd(new Cmd_run     (&sys, &shell));

#if 0
    ssnt_txrxwk txrxwk(&sys);
    sys.cpus[1].thrd = &shell;
    sys.cpus[2].thrd = &txrxwk;
#else
    ssnt_rx rx(&sys);
    ssnt_tx tx(&sys);
    ssnt_wk wk(&sys, 40000);
    sys.cpus[1].thrd = &shell;
    sys.cpus[2].thrd = &rx;
    sys.cpus[3].thrd = &tx;
    sys.cpus[4].thrd = &wk;
    sys.cpus[5].thrd = &wk;
    sys.cpus[6].thrd = &wk;
    sys.cpus[8].thrd = &wk;
    sys.cpus[9].thrd = &wk;
    sys.cpus[10].thrd = &wk;
    sys.cpus[11].thrd = &wk;
#endif

    sys.cpus[1].launch();
    sys.wait_all();
}

