

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include <stcp/ifaddr.h>
#include <stcp/config.h>


namespace slank {
    


const char* af2str(af_t af)
{
    switch (af) {
        case af_link: return "AF_LINK";
        case af_inet: return "AF_INET";
        default : return "unknown";
    }
}

void ifaddr::init(const void* d, size_t l)
{
    log& log = log::instance();
    log.push(af2str(family));

    switch (family) {
        case af_inet:
            {
                fprintf(stderr, "Not Impl yet\n");
                exit(-1);
                break;
            }
        case af_link:
            {
                if (l != ETHER_ADDR_LEN) {
                    fprintf(stderr, "Invalid Address len\n");
                    exit(-1);
                }
                memcpy(&raw.link, d, l);
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


} /* namespace */
