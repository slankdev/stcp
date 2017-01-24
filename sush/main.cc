
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <string>

#include <readline/readline.h>
#include <readline/history.h>

#include <slankdev/socketfd.h>
#include "cmd.h"



char* Readline(const char* p)
{
    char* line = readline(p);
    add_history(line);
    return line;
}


class Socket : public slankdev::socketfd {
public:
    void send_cmd(cmd_type type)
    {
        struct cmd_hdr hdr;
        memset(&hdr, 0, sizeof(hdr));
        hdr.type = type;
        // hdr.len  = 0xeeee;

        const char data[] = "adfdfdfdfdfdf";
        // write(data, sizeof(data));
        write(&hdr, sizeof(hdr));
    }
};





int main(int argc, char** argv)
{
    Socket sock;
    sock.socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port   = htons(atoi(argv[1]));
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    sock.connect((sockaddr*)&server, sizeof(server));

    const char* prmpt = "SUSANOO$ ";
    bool runnning = true;
    while (char* line = Readline(prmpt)) {

        if (strcmp(line, "")==0) {
        } else if (strcmp(line, "help")==0) {
            sock.send_cmd(CMD_HELP);
        } else if (strcmp(line, "quit")==0) {
            sock.send_cmd(CMD_QUIT);
            runnning = false;
        } else if (strcmp(line, "get")==0) {
            sock.send_cmd(CMD_GET);
        } else if (strcmp(line, "set")==0) {
            sock.send_cmd(CMD_SET);
        } else {
            printf("unknown command\n");
        }

        free(line);
        if (!runnning) break;
    }
}



