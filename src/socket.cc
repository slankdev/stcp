


#include <stcp/ifnet.h>
#include <stcp/dpdk.h>
#include <stcp/rte.h>
#include <stcp/stcp.h>
#include <stcp/socket.h>

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
    int32_t o[4];
    int ret = sscanf(fmt, "%d.%d.%d.%d", &o[0], &o[1], &o[2], &o[3]);
    if (ret != 4)
        throw slankdev::exception("invalid format");

    for (int i=0; i<4; i++) {
        if (o[i] < 0 || 255 < o[i])
            throw slankdev::exception("invalid format");
    }

    return stcp_inet_addr(
       uint8_t(o[0]),
       uint8_t(o[1]),
       uint8_t(o[2]),
       uint8_t(o[3]));
}


char* p_sockaddr_to_str(const struct stcp_sockaddr* sa)
{
    static char str[16];
    const stcp_sockaddr_in* sin = reinterpret_cast<const stcp_sockaddr_in*>(sa);
    sprintf(str, "%d.%d.%d.%d", 
            sin->sin_addr.addr_bytes[0], sin->sin_addr.addr_bytes[1],
            sin->sin_addr.addr_bytes[2], sin->sin_addr.addr_bytes[3]);
    return str;
}
char* hw_sockaddr_to_str(const struct stcp_sockaddr* sa)
{
    static char str[32];
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", 
            sa->sa_data[0], sa->sa_data[1],
            sa->sa_data[2], sa->sa_data[3],
            sa->sa_data[4], sa->sa_data[5]);
    return str;
}

} /* namespace slank */
