

#pragma once

#include <vector>
#include <string>
#include "command.h"


class Cmd_version : public Command {
public:
    Cmd_version() { name = "version"; }
    void operator()()
    {
        printf("Susanoo version 0.0 \n");
    }
};


class Cmd_quit : public Command {
public:
    Cmd_quit() { name = "quit"; }
    void operator()()
    {
        exit(-1);
    }
};


class Cmd_ifconfig : public Command {
public:
    Cmd_ifconfig() { name = "ifconfig"; }
    void operator()()
    {
        printf("PORT0 \n");
    }
};


