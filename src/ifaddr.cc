

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include <stcp/ifaddr.h>
#include <stcp/config.h>
#include <stcp/types.h>


namespace slank {
    

struct stcp_in_addr stcp_inet_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4)
{
    stcp_in_addr a;
    a.addr_bytes[0] = o1;
    a.addr_bytes[1] = o2;
    a.addr_bytes[2] = o3;
    a.addr_bytes[3] = o4;
    return a;
}


struct stcp_in_addr stcp_inet_addr(const char* fmt)
{
    int o[4];
    int ret = sscanf(fmt, "%d.%d.%d.%d", &o[0], &o[1], &o[2], &o[3]);
    if (ret != 4)
        throw slankdev::exception("invalid format");

    for (int i=0; i<4; i++) {
        if (o[i] < 0 || 255 < o[i])
            throw slankdev::exception("invalid format");
    }

    return stcp_inet_addr(
       static_cast<uint8_t>(o[0]),
       static_cast<uint8_t>(o[1]),
       static_cast<uint8_t>(o[2]),
       static_cast<uint8_t>(o[3])
    );
}

const char* af2str(stcp_sa_family af)
{
    switch (af) {
        case STCP_AF_LINK: return "AF_LINK";
        case STCP_AF_INET: return "AF_INET";
        default : return "unknown";
    }
}

} /* namespace */
