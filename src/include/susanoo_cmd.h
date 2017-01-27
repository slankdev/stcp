

#include <susanoo.h>

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>



class Command {
public:
    std::string name;
    virtual void operator()(const std::vector<std::string>& args) = 0;
    virtual ~Command() {}
};

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
    System* sys;
public:
    Cmd_quit(System* s) : sys(s) { name = "quit"; }
    void operator()(const std::vector<std::string>& args)
    {
        UNUSED(args);
        sys->halt();
    }
};


class Cmd_lscpu : public Command {
    System* sys;

    byteorder get_byteorder()
    {
        uint16_t before = 0x1234;
        uint16_t after  = rte_le_to_cpu_16(before);
        if (before == after) return S_LITTLE_ENDIAN;
        else                 return S_BIG_ENDIAN;
    }
public:
    Cmd_lscpu(System* s) : sys(s) { name = "lscpu"; }
    void operator()(const std::vector<std::string>& args)
    {
        UNUSED(args);

        printf("Architecture        : \n");
        printf("CPU op-mode(s)      : \n");
        printf("Byte Order          : %s \n",byteorder2str(get_byteorder()));
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
};

// class Cmd_ : public Command {
//     System* sys;
// public:
//     Cmd_quit(System* s) : sys(s) { name = "quit"; }
//     void operator()(const std::vector<std::string>& args)
//     {
//         UNUSED(args);
//         sys->halt();
//     }
// };
// class Cmd_reboot
// class Cmd_show
//    show config
//    show route
//    show stats
//    show statistic
// class Cmd_port
//    port 0 linkdown
//    port 0 linkup
//    port 0 blink
// class Cmd_commit
// class Cmd_export
// class Cmd_inport







static inline char* Readline(const char* p)
{
    char* line = readline(p);
    add_history(line);
    return line;
}

inline Shell::Shell(System* s) : sys(s)
{
    add_cmd(new Cmd_version()   );
    add_cmd(new Cmd_quit   (sys));
    add_cmd(new Cmd_lscpu  (sys));
}
inline Shell::~Shell() { for (Command* cmd : cmds) delete(cmd); }
inline void Shell::add_cmd(Command* newcmd)
{
    cmds.push_back(newcmd);
}
inline void Shell::exe_cmd(const char* cmd_str)
{
    if (strlen(cmd_str) == 0) return;
    std::vector<std::string> args = slankdev::split(cmd_str, ' ');
    for (Command* cmd : cmds) {
        if (cmd->name == args[0]) {
            (*cmd)(args);
            return;
        }
    }
    printf("SUSH: command not found: %s\n", args[0].c_str());
}
inline void Shell::operator()()
{
    printf("\n\n");
    const char* prmpt = "SUSANOO$ ";
    while (char* line = Readline(prmpt)) {
        exe_cmd(line);
        free(line);
    }
    return;
}
