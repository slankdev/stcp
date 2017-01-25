

#include <stdio.h>
#include <dpdk/system.h>
#include <susanoo_shell.h>
#include <usr_cmds.h>
#include <usr_thrds.h>


int main(int argc, char** argv)
{
    dpdk::System::rx_ring_size   = 128;
    dpdk::System::tx_ring_size   = 512;
    dpdk::System::port_bulk_size = 32;

    dpdk::System sys(argc, argv);
    if (sys.ports.size()%2 != 0) return -1;

    sys.shell.add_cmd(new Cmd_version ()   );
    sys.shell.add_cmd(new Cmd_quit    (&sys));
    sys.shell.add_cmd(new Cmd_ifconfig(&sys));
    sys.shell.add_cmd(new Cmd_test    (&sys));

    ssnt_txrxwk t2(&sys);
    sys.cpus[1].thrd = &sys.shell;
    sys.cpus[2].thrd = &t2;

    sys.launch();
}

