
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

#include <ssnlib_sys.h>
#include <ssnlib_cmd.h>

namespace ssnlib {


class Cmd_version : public Command {
public:
    Cmd_version() { name = "version"; }
    void operator()(const std::vector<std::string>& args)
    {
        UNUSED(args);
        printf("Susanoo version 0.0 \n");
    }
};

class Cmd_kill : public Command {
    System* sys;
public:
    Cmd_kill(System* s) : sys(s) { name = "kill"; }
    void operator()(const std::vector<std::string>& args)
    {
        if (args.size() < 2) {
            fprintf(stderr, "Usage: %s lcore_id \n", args[0].c_str());
            return ;
        }

        uint8_t lcore_id = atoi(args[1].c_str());
        printf("kill lcore%d ... ", lcore_id);
        fflush(stdout);

        rte_lcore_state_t state = sys->cpus[lcore_id].get_state();
        if (state != RUNNING) {
            fprintf(stderr, "Error: lcore%d is not runnning \n", lcore_id);
            return ;
        }

        sys->cpus[lcore_id].thrd->kill();
        rte_eal_wait_lcore(lcore_id);
        printf("done \n");
    }
};

class Cmd_launch : public Command {
    System* sys;
public:
    Cmd_launch(System* s) : sys(s) { name = "launch"; }
    void operator()(const std::vector<std::string>& args)
    {
        if (args.size() < 2) {
            fprintf(stderr, "Usage: %s lcore_id \n", args[0].c_str());
            return ;
        }

        uint8_t lcore_id = atoi(args[1].c_str());
        printf("luanch lcore%d ... ", lcore_id);
        fflush(stdout);

        rte_lcore_state_t state = sys->cpus[lcore_id].get_state();
        if (state == RUNNING) {
            fprintf(stderr, "Error: lcore%d was already launched \n", lcore_id);
            return ;
        }

        sys->cpus[lcore_id].launch();
        printf("done \n");
    }
};


class Cmd_quit : public Command {
    System* sys;
public:
    Cmd_quit(System* s) : sys(s) { name = "quit"; }
    void operator()(const std::vector<std::string>& args)
    {
        UNUSED(args);
        sys->halt();
    }
};


class Cmd_lscpu : public Command {
    System* sys;
public:
    Cmd_lscpu(System* s) : sys(s) { name = "lscpu"; }
    void operator()(const std::vector<std::string>& args)
    {
        UNUSED(args);

        printf("Architecture        : \n");
        printf("CPU op-mode(s)      : \n");

        uint16_t before = 0x1234;
        uint16_t after  = rte_le_to_cpu_16(before);
        printf("Byte Order          : %s \n",before==after?"Little Endian":"Big Endian");

        printf("CPU(s):             : \n");
        printf("On-line CPU(s) list : \n");
        printf("Thread(s) per core  : \n");
        printf("Core(s) per socket  : \n");
        printf("Socket(s)           : \n");
        printf("NUMA node(s)        : \n");
        printf("Vendor ID           : \n");
        printf("CPU family          : \n");
        printf("Model               : \n");
        printf("Model name          : \n");
        printf("Stepping            : \n");
        printf("CPU MHz             : \n");
        printf("CPU max MHz         : \n");
        printf("CPU min MHz         : \n");
        printf("BogoMIPS            : \n");
        printf("Virtualization      : \n");
        printf("L1d cache           : \n");
        printf("L1i cache           : \n");
        printf("L2 cache            : \n");
        printf("L3 cache            : \n");
        printf("NUMA node0 CPU(s)   : \n");

        printf("Flags               : ");
        for (size_t i=0; i<RTE_CPUFLAG_NUMFLAGS; i++) {
            if (sys->cpuflags[i])
                printf("%s ", rte_cpu_get_flag_name(rte_cpu_flag_t(i)));
        }
        printf("\n");
    }
};



} /* namespace ssnlib */





