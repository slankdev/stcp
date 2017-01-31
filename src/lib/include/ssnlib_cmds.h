
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






class Cmd_show : public Command {
    System* sys;
public:
    Cmd_show(System* s) : sys(s) { name = "show"; }

    void show_port()
    {
        for (auto& port : sys->ports) {
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
                size_t rxqsize  = port.rxq[qid].size();
                size_t rxqcount = port.rxq[qid].count();
                printf("  RX ring%u:%zd/%zd \n", qid,
                        rxqcount, rxqsize);
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
        } else {
            usage(args[0]);
        }
    }
};

class Cmd_thread : public  Command {
    System* sys;
public:
    Cmd_thread(System* s) : sys(s) { name = "thread"; }
    void launch(size_t lcore_id)
    {
        rte_lcore_state_t state = sys->cpus[lcore_id].get_state();
        if (state == RUNNING) {
            fprintf(stderr, "Error: lcore%zd was already launched \n", lcore_id);
            return ;
        }
        sys->cpus[lcore_id].launch();
    }
    void kill(size_t lcore_id)
    {
        rte_lcore_state_t state = sys->cpus[lcore_id].get_state();
        if (state != RUNNING) {
            fprintf(stderr, "Error: lcore%zd is not runnning \n", lcore_id);
            return ;
        }
        bool ret = sys->cpus[lcore_id].thread->kill();
        if (ret) {
            rte_eal_wait_lcore(lcore_id);
            printf("done \n");
        } else {
            printf("can't killed \n");
        }
    }
    void show()
    {
        for (ssnlib::Cpu& cpu : sys->cpus) {
            printf("lcore%u thread status: %s \n", cpu.lcore_id,
                ssnlib::util::rte_lcore_state_t2str(rte_eal_get_lcore_state(cpu.lcore_id)));
        }
    }
    void usage(const std::string s)
    {
        fprintf(stderr, "Usage: %s { launch id | kill id | show } \n", s.c_str());
    }
    void operator()(const std::vector<std::string>& args)
    {
        if (args.size() >= 3) {
            uint8_t lcore_id = atoi(args[2].c_str());
            if (args[1] == "launch") {
                launch(lcore_id);
            } else if (args[1] == "kill") {
                kill(lcore_id);
            } else {
                usage(args[0]);
            }
            return ;
        } else if (args.size() >= 2) {
            if (args[1] == "show") {
                show();
                return ;
            }
        }
        usage(args[0]);
        return ;
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





