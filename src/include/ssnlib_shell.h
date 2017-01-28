
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>

#include <susanoo.h>
#include <ssnlib_thread.h>
#include <ssnlib_cmd.h>
#include <ssnlib_cmds.h>


static inline char* Readline(const char* p)
{
    char* line = readline(p);
    add_history(line);
    return line;
}

namespace ssnlib {

class Shell : public ssnlib::ssn_thread {
    std::vector<ssnlib::Command*> cmds;
    System* sys;
public:

    // Shell(System* s);
    // ~Shell();
    // void add_cmd(ssnlib::Command* newcmd);
    // void exe_cmd(const char* cmd_str);
    // void operator()();


    Shell(System* s) : sys(s)
    {
        add_cmd(new ssnlib::Cmd_version()   );
        add_cmd(new ssnlib::Cmd_quit   (sys));
        add_cmd(new ssnlib::Cmd_lscpu  (sys));
    }

    ~Shell() { for (ssnlib::Command* cmd : cmds) delete(cmd); }

    void add_cmd(ssnlib::Command* newcmd)
    {
        cmds.push_back(newcmd);
    }


    void exe_cmd(const char* cmd_str)
    {
        if (strlen(cmd_str) == 0) return;
        std::vector<std::string> args = slankdev::split(cmd_str, ' ');
        for (ssnlib::Command* cmd : cmds) {
            if (cmd->name == args[0]) {
                (*cmd)(args);
                return;
            }
        }
        printf("SUSH: command not found: %s\n", args[0].c_str());
    }


    void operator()()
    {
        printf("\n\n");
        const char* prmpt = "SUSANOO$ ";
        while (char* line = Readline(prmpt)) {
            exe_cmd(line);
            free(line);
        }
        return;
    }
};



} /* namespace ssnlib */

