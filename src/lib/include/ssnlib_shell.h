
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>

#include <ssnlib_sys.h>
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

    Shell(System* s) : sys(s)
    {
        add_cmd(new Cmd_clear  ()   );
        add_cmd(new Cmd_quit   (sys));
        add_cmd(new Cmd_thread (sys));
        add_cmd(new Cmd_show   (sys));
    }

    ~Shell() { for (ssnlib::Command* cmd : cmds) delete(cmd); } // TODO

    void help()
    {
        printf("Commands: \n");
        for (const Command* c : cmds) {
            printf("  %s \n", c->name.c_str());
        }
    }

    bool kill() { return false; }

    void add_cmd(ssnlib::Command* newcmd)
    {
        printf("SHELL: add command [%s]\n", newcmd->name.c_str());
        cmds.push_back(newcmd);
    }


    void exe_cmd(const char* cmd_str)
    {
        if (strlen(cmd_str) == 0) return;
        std::vector<std::string> args = slankdev::split(cmd_str, ' ');
        if (args[0] == "help") {
            help();
        } else {
            for (ssnlib::Command* cmd : cmds) {
                if (cmd->name == args[0]) {
                    (*cmd)(args);
                    return;
                }
            }
            printf("SUSH: command not found: %s\n", args[0].c_str());
        }
    }


    void operator()()
    {
        const char* prmpt = "SUSANOO$ ";
        while (char* line = Readline(prmpt)) {
            exe_cmd(line);
            free(line);
        }
        return;
    }
};



} /* namespace ssnlib */

