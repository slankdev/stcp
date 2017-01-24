

#pragma once
#include <stdint.h>
#include <stddef.h>


enum cmd_type : uint32_t {
    CMD_QUIT,
    CMD_HELP,
    CMD_GET,
    CMD_SET,
    CMD_ANS,
};


enum cmd_code : uint32_t {
};


struct cmd_hdr {
    cmd_type type; // 32bit
    cmd_code code; // 32bit
    uint32_t data_len;
};


const char* cmd_type2str(cmd_type t)
{
    switch (t) {
        case CMD_QUIT: return "QUIT";
        case CMD_HELP: return "HELP";
        case CMD_GET : return "GET ";
        case CMD_SET : return "SET ";
        case CMD_ANS : return "ANS ";
        default: return "UNKNOWN_ERROR";
    }
}


