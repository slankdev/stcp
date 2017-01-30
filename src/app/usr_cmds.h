

#pragma once

#include <vector>
#include <string>

#include <ssnlib_sys.h>
#include <ssnlib_cmd.h>




class Cmd_test : public ssnlib::Command {
    ssnlib::System* sys;
    ssnlib::Shell*  shell;
public:
    Cmd_test(ssnlib::System* s, ssnlib::Shell* sh)
        : sys(s), shell(sh) { name = "test"; }
    void operator()(const std::vector<std::string>& args)
    {
        UNUSED(args);
        for (;;) {
            slankdev::clear_screen();
            shell->exe_cmd("show port");
            usleep(50000);
        }
    }
};


class Cmd_run : public ssnlib::Command {
    ssnlib::System* sys;
    ssnlib::Shell*  shell;
public:
    Cmd_run(ssnlib::System* s, ssnlib::Shell* sh)
        : sys(s), shell(sh) { name = "run"; }
    void operator()(const std::vector<std::string>& args)
    {
        UNUSED(args);
        shell->exe_cmd("thrd launch 2");
        shell->exe_cmd("thrd launch 3");
        shell->exe_cmd("thrd launch 4");
    }
};




