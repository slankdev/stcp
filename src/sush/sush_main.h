

#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "command.h"
#include "usr_cmds.h"
#include "sush.h"


int thread_sush(void* arg)
{
    printf("\n\n"); (void)(arg);
    // dpdk::System* sys = reinterpret_cast<dpdk::System*>(arg);

    sush sys;
    sys.add_cmd(new Cmd_quit());
    sys.add_cmd(new Cmd_version());
    sys.add_cmd(new Cmd_ifconfig());
    return sys.main_loop();
}





