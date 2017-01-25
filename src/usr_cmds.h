

#pragma once

#include <vector>
#include <string>

#include <susanoo.h>

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
        printf("  RX ring:%zd/%zd TX ring:%zd/%zd\n",
                port.rxq[0].count(), port.rxq[0].size(),
                port.txq[0].count(), port.txq[0].size());
    }
}



class Cmd_ifconfig : public System::Command {
    System* sys;
public:
    Cmd_ifconfig(System* s) : sys(s) { name = "ifconfig"; }
    void operator()(const std::vector<std::string>& args)
    {
        UNUSED(args);
        ifconfig(sys);
    }
};


class Cmd_test : public System::Command {
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


