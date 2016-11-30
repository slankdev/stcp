


#include <stcp/ifnet.h>
#include <stcp/dpdk.h>
#include <stcp/stcp.h>
#include <stcp/socket.h>

#include <string>

namespace slank {


const stcp_ether_addr stcp_ether_addr::broadcast(0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
const stcp_ether_addr stcp_ether_addr::zero(0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

const stcp_in_addr    stcp_in_addr::broadcast(0xff, 0xff, 0xff, 0xff);
const stcp_in_addr    stcp_in_addr::zero(0x00, 0x00, 0x00, 0x00);


bool stcp_sockaddr::operator==(const stcp_sockaddr& rhs) const
{
    if (sa_fam != rhs.sa_fam)
        return false;

    switch (sa_fam) {
        case STCP_AF_LINK:
        {
            for (size_t i=0; i<stcp_ether_addr::addrlen; i++) {
                if (sa_data[i] != rhs.sa_data[i])
                    return false;
            }
            return true;
            break;
        }
        case STCP_AF_INET:
        {
            for (size_t i=0; i<stcp_in_addr::addrlen; i++) {
                if (sa_data[i] != rhs.sa_data[i])
                    return false;
            }
            return true;
            break;
        }
        default:
        {
            throw exception("sorry not impl yet");
            break;
        }
    }

}



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

    for (size_t i=0; i<stcp_ether_addr::addrlen; i++) {
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
    for (size_t i=0; i<stcp_in_addr::addrlen; i++) {
        if (sin->sin_addr.addr_bytes[i] != addr.addr_bytes[i])
            return false;
    }
    return true;
}
bool operator!=(const stcp_sockaddr& sa, const stcp_in_addr& addr)
{
    return !(sa == addr);
}




void stcp_sockaddr::inet_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4)
{
    stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(this);
    sin->sin_fam = STCP_AF_INET;
    sin->sin_addr.set(o1, o2, o3, o4);
}
void stcp_sockaddr::inet_hwaddr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t o5, uint8_t o6)
{
    sa_fam = STCP_AF_LINK;
    sa_data[0] = o1;
    sa_data[1] = o2;
    sa_data[2] = o3;
    sa_data[3] = o4;
    sa_data[4] = o5;
    sa_data[5] = o6;
}
void stcp_sockaddr_in::inet_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4)
{
    sin_addr.set(o1, o2, o3, o4);
}

} /* namespace slank */
