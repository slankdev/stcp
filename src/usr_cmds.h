

#pragma once

#include <vector>
#include <string>




static void ifconfig(dpdk::System* sys)
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


class Cmd_version : public Command {
public:
    Cmd_version() { name = "version"; }
    void operator()(const std::vector<std::string>& args)
    {
        UNUSED(args);
        printf("Susanoo version 0.0 \n");
    }
};


class Cmd_quit : public Command {
    dpdk::System* sys;
public:
    Cmd_quit(dpdk::System* s) : sys(s) { name = "quit"; }
    void operator()(const std::vector<std::string>& args)
    {
        UNUSED(args);
        sys->halt();
    }
};


class Cmd_ifconfig : public Command {
    dpdk::System* sys;
public:
    Cmd_ifconfig(dpdk::System* s) :sys(s) { name = "ifconfig"; }
    void operator()(const std::vector<std::string>& args)
    {
        UNUSED(args);
        ifconfig(sys);
    }
};


class Cmd_test : public Command {
    dpdk::System* sys;
public:
    Cmd_test(dpdk::System* s) :sys(s) { name = "test"; }
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


