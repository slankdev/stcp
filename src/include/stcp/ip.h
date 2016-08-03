
#pragma once

#include <stcp/protocol.h>



using ipaddr_t = uint32_t;

inline
void uint32_t_to_char(ipaddr_t ip, uint8_t a[4])
{
    a[0] = (unsigned char)(ip >> 24 & 0xff);
    a[1] = (unsigned char)(ip >> 16 & 0xff);
    a[2] = (unsigned char)(ip >> 8 & 0xff);
    a[3] = (unsigned char)(ip & 0xff);
}


