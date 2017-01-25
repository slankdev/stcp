

#pragma once

#include <vector>
#include <string>

#include "command.h"
#include <dpdk/api.h>


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
        api::ifconfig(sys);
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
            api::ifconfig(sys);
            usleep(50000);
        }
    }
};
