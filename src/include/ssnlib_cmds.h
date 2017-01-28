
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

#include <slankdev/util.h>

#include <ssnlib_sys.h>
#include <ssnlib_cmd.h>

namespace ssnlib {



class Cmd_clear : public Command {
public:
    Cmd_clear() { name = "clear"; }
    void operator()(const std::vector<std::string>& args)
    {
        UNUSED(args);
        slankdev::clear_screen();
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

        bool ret = sys->cpus[lcore_id].thrd->kill();
        if (ret) {
            rte_eal_wait_lcore(lcore_id);
            printf("done \n");
        } else {
            printf("can't killed \n");
        }
    }
};




class Cmd_show : public Command {
    System* sys;
public:
    Cmd_show(System* s) : sys(s) { name = "show"; }

    void show_port()
    {
        for (ssnlib::Port& port : sys->ports) {
            port.stats.update();

            printf("%s\n", port.name.c_str());
            printf("  HWaddr %s \n", port.addr.toString().c_str());

            auto& stats = port.stats;
            printf("  RX packets:%lu errors:%lu dropped:%lu allocmiss:%lu \n",
                        stats.raw.ipackets, stats.raw.ierrors,
                        stats.raw.imissed, stats.raw.rx_nombuf);
            printf("  TX packets:%lu errors:%lu  \n",
                    stats.raw.opackets, stats.raw.oerrors);
            printf("  RX bytes:%lu TX bytes:%lu \n", stats.raw.ibytes, stats.raw.obytes);



            size_t nb_rxq = port.rxq.size();
            size_t nb_txq = port.txq.size();
            for (uint8_t qid=0; qid<nb_rxq; qid++) {
                printf("  RX%u packets:%lu errors:%lu ", qid,
                        stats.raw.q_ipackets[qid], stats.raw.q_errors[qid]);
                printf("  RX ring%u:%zd/%zd \n", qid,
                        port.rxq[qid].count(), port.rxq[qid].size());
            }
            printf("\n");
            for (uint8_t qid=0; qid<nb_txq; qid++) {
                printf("  TX%u packets:%lu ", qid, stats.raw.q_opackets[qid]);
                printf("  TX ring%u:%zd/%zd \n", qid,
                        port.txq[qid].count(), port.txq[qid].size());
            }
            printf("\n");
        }
    }
    void show_cpu()
    {
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
    void show_version()
    {
        printf("Susanoo version 0.0 \n");
    }
    void show_thread()
    {
        for (ssnlib::Cpu& cpu : sys->cpus) {
            printf("lcore%u thread status: %s \n", cpu.lcore_id,
                ssnlib::util::rte_lcore_state_t2str(rte_eal_get_lcore_state(cpu.lcore_id)));
        }
    }
    void usage(const std::string& s)
    {
        printf("Usage: %s COMMAND \n", s.c_str());
        printf(" COMMAND := { port | cpu | version | thread } \n");
    }
    void operator()(const std::vector<std::string>& args)
    {
        if (args.size() < 2) {
            usage(args[0]);
            return ;
        }

        if (args[1] == "port") {
            show_port();
        } else if (args[1] == "cpu") {
            show_cpu();
        } else if (args[1] == "version") {
            show_version();
        } else if (args[1] == "thread") {
            show_thread();
        } else {
            usage(args[0]);
        }
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
        fflush(stdout);

        rte_lcore_state_t state = sys->cpus[lcore_id].get_state();
        if (state == RUNNING) {
            fprintf(stderr, "Error: lcore%d was already launched \n", lcore_id);
            return ;
        }

        sys->cpus[lcore_id].launch();
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



} /* namespace ssnlib */





