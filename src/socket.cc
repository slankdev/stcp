


#include <stcp/ifnet.h>
#include <stcp/dpdk.h>
#include <stcp/rte.h>
#include <stcp/stcp.h>
#include <stcp/socket.h>

#include <string>

namespace slank {



const char* stcp_sockaddr::c_str() const
{
    static char str[32];
    switch (sa_fam) {
        case STCP_AF_LINK:
            {
                sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", 
                        sa_data[0], sa_data[1],
                        sa_data[2], sa_data[3],
                        sa_data[4], sa_data[5]);
                break;
            }
        case STCP_AF_INET:
            {
                const stcp_sockaddr_in* sin = reinterpret_cast<const stcp_sockaddr_in*>(this);
                sprintf(str, "%d.%d.%d.%d", 
                        sin->sin_addr.addr_bytes[0], sin->sin_addr.addr_bytes[1],
                        sin->sin_addr.addr_bytes[2], sin->sin_addr.addr_bytes[3]);
                break;
            }
        default:
            {
                std::string errstr = "Address Family is not supported ";
                errstr += std::to_string(sa_fam);
                throw exception(errstr.c_str());
                break;
            }
    }
    return str;
}


const char* stcp_sockaddr_in::c_str() const
{
    static char str[32];
    const stcp_sockaddr_in* sin = reinterpret_cast<const stcp_sockaddr_in*>(this);
    sprintf(str, "%d.%d.%d.%d", 
            sin->sin_addr.addr_bytes[0], sin->sin_addr.addr_bytes[1],
            sin->sin_addr.addr_bytes[2], sin->sin_addr.addr_bytes[3]);
    return str;
}

bool operator==(const stcp_sockaddr& sa, const stcp_ether_addr& addr)
{
    if (sa.sa_fam != STCP_AF_LINK)
        return false;

    for (int i=0; i<6; i++) {
        if (sa.sa_data[i] != addr.addr_bytes[i])
            return false;
    }
    return true;
}
bool operator!=(const stcp_sockaddr& sa, const stcp_ether_addr& addr)
{
    return !(sa == addr);
}
bool operator==(const stcp_sockaddr& sa, const stcp_in_addr& addr)
{
    if (sa.sa_fam != STCP_AF_INET)
        return false;

    const stcp_sockaddr_in* sin = reinterpret_cast<const stcp_sockaddr_in*>(&sa);
    for (int i=0; i<4; i++) {
        if (sin->sin_addr.addr_bytes[i] != addr.addr_bytes[i])
            return false;
    }
    return true;
}
bool operator!=(const stcp_sockaddr& sa, const stcp_in_addr& addr)
{
    return !(sa == addr);
}



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
        throw exception("invalid format");

    for (int i=0; i<4; i++) {
        if (o[i] < 0 || 255 < o[i])
            throw exception("invalid format");
    }

    return stcp_inet_addr(
       uint8_t(o[0]),
       uint8_t(o[1]),
       uint8_t(o[2]),
       uint8_t(o[3]));
}

struct stcp_sockaddr stcp_inet_hwaddr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t o5, uint8_t o6)
{
    stcp_sockaddr sa(STCP_AF_LINK);
    memset(&sa, 0, sizeof sa);

    sa.sa_fam = STCP_AF_LINK;
    sa.sa_data[0] = o1;
    sa.sa_data[1] = o2;
    sa.sa_data[2] = o3;
    sa.sa_data[3] = o4;
    sa.sa_data[4] = o5;
    sa.sa_data[5] = o6;

    return sa;
}




void stcp_sockaddr::inet_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4)
{
    stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(this);
    sin->sin_fam = STCP_AF_INET;
    sin->sin_addr = stcp_inet_addr(o1, o2, o3, o4);
}
void stcp_sockaddr::inet_hwaddr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t o5, uint8_t o6)
{
    sa_fam = STCP_AF_LINK;
    *this = stcp_inet_hwaddr(o1, o2, o3, o4, o5, o6);
}



void stcp_sockaddr_in::inet_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4)
{
    sin_addr = stcp_inet_addr(o1, o2, o3, o4);
}

} /* namespace slank */
