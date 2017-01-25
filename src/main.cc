

#include <stdio.h>
#include <susanoo.h>
#include <susanoo_shell.h>
#include <usr_cmds.h>
#include <usr_thrds.h>


int main(int argc, char** argv)
{
    System::rx_ring_size   = 128;
    System::tx_ring_size   = 512;
    System::port_bulk_size = 32;

    System sys(argc, argv);
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

