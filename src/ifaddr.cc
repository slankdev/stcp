

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include <stcp/ifaddr.h>
#include <stcp/config.h>
#include <stcp/types.h>


namespace slank {
    


const char* af2str(stcp_sa_family af)
{
    switch (af) {
        case STCP_AF_LINK: return "AF_LINK";
        case STCP_AF_INET: return "AF_INET";
        default : return "unknown";
    }
}

void ifaddr::init(const void* d, size_t l)
{
    switch (family) {
        case STCP_AF_INET:
            {
                fprintf(stderr, "Not Impl yet\n");
                exit(-1);
                break;
            }
        case STCP_AF_LINK:
            {
                if (l != STCP_ETHER_ADDR_LEN) {
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
}


} /* namespace */
