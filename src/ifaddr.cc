

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>

#include <string>
#include <iostream>
#include <sstream>
#include <exception>
#include <vector>

#include <stcp/rte.h>
#include <stcp/config.h>
#include <stcp/ifaddr.h>


static char* af2str(af_t af)
{
    static char str[8];
    switch (af) {
        case AF_INET:
            strcpy(str, "INET");
            break;
        case AF_LINK:
            strcpy(str, "LINK");
            break;
        default:
            strcpy(str, "UNKNOWN");
            break;
    }
    return str;
}

void ifaddr::init(const void* d, size_t l)
{
    log& log = log::instance();
    log.push(af2str(family));

    switch (family) {
        case AF_INET:
            {
                fprintf(stderr, "Not Impl yet\n");
                exit(-1);
                break;
            }
        case AF_LINK:
            {
                if (l != ETHER_ADDR_LEN) {
                    fprintf(stderr, "Invalid Address len\n");
                    exit(-1);
                }
                memcpy(&raw.link, d, l);

                char str[32];
                sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", 
                        raw.link.addr_bytes[0], raw.link.addr_bytes[1], 
                        raw.link.addr_bytes[2], raw.link.addr_bytes[3], 
                        raw.link.addr_bytes[3], raw.link.addr_bytes[5]);
                log.write(INFO, "set address %s", str);

                break;
            }
        default:
            {
                fprintf(stderr, "Unknown address family\n");
                exit(-1);
                break;
            }
    }
    log.pop();
}
