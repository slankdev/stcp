

#pragma once

#include <vector>
#include <string>

#include <susanoo.h>
#include <susanoo_cmd.h>

static void ifconfig(System* sys)
{
    for (dpdk::Port& port : sys->ports) {
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



class Cmd_ifconfig : public Command {
    System* sys;
public:
    Cmd_ifconfig(System* s) : sys(s) { name = "ifconfig"; }
    void operator()(const std::vector<std::string>& args)
    {
        UNUSED(args);
        ifconfig(sys);
    }
};


class Cmd_test : public Command {
    System* sys;
public:
    Cmd_test(System* s) : sys(s) { name = "test"; }
    void operator()(const std::vector<std::string>& args)
    {
        UNUSED(args);
        for (;;) {
            slankdev::clear_screen();
            ifconfig(sys);
            usleep(50000);
        }
    }
};


