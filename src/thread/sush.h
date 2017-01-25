
#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "sush/command.h"
#include "sush/usr_cmds.h"
#include "sush/sush.h"


int thread_sush(void* arg)
{
    dpdk::System* sys = reinterpret_cast<dpdk::System*>(arg);
    printf("\n\n");

    sush sush0;
    sush0.add_cmd(new Cmd_version ()   );
    sush0.add_cmd(new Cmd_quit    (sys));
    sush0.add_cmd(new Cmd_ifconfig(sys));
    sush0.add_cmd(new Cmd_test    (sys));
    return sush0.main_loop();
}
