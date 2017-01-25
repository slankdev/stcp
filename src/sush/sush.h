
#pragma once

#include <vector>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>

#include "command.h"


char* Readline(const char* p)
{
    char* line = readline(p);
    add_history(line);
    return line;
}


class sush {
    friend class Command;
    std::vector<Command*> cmds;
public:
    ~sush() { for (Command* cmd : cmds) delete(cmd); }
    void add_cmd(Command* newcmd)
    {
        cmds.push_back(newcmd);
    }
    void exe_cmd(const char* cmd_str)
    {
        if (strlen(cmd_str) == 0) return;
        for (Command* cmd : cmds) {
            if (cmd->name == cmd_str) {
                (*cmd)();
                return;
            }
        }
        printf("SUSH: command not found: %s\n", cmd_str);
    }
    int main_loop()
    {
        const char* prmpt = "SUSANOO$ ";
        while (char* line = Readline(prmpt)) {
            exe_cmd(line);
            free(line);
        }
        return 0;
    }
};

