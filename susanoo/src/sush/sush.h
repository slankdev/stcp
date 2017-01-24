

#pragma once
#include <slankdev/socketfd.h>
#include <slankdev/util.h>
#include "cmd.h"



class Socket : public slankdev::socketfd {
public:
    Socket() {}
    Socket(int fd) : slankdev::socketfd(fd) {}
    void proc_querry(void* buf, size_t len)
    {
        cmd_hdr* hdr = reinterpret_cast<cmd_hdr*>(buf);
        printf("cmd-type: %s \n", cmd_type2str(hdr->type));
    }
};



#define PORT 1111
int thread_sush(void* arg)
{
    UNUSED(arg);

    Socket sock;
    sock.socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    sock.bind((sockaddr*)&addr, sizeof(addr));

    sock.listen(1);

    while (1) {
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        Socket client_sock = sock.accept((sockaddr*)&client, &client_len);
        printf("accept new socket \n");

        while (1) {
            uint8_t buf[1000];
            size_t recvlen = client_sock.read(buf, sizeof(buf));
            if (recvlen == 0) break;
            if (recvlen < sizeof(cmd_hdr)) break;

            client_sock.proc_querry(buf, recvlen);
        }
    }
    return 0;
}




