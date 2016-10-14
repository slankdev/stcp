
#pragma once

#include <stcp/config.h>
#include <stcp/rte.h> // struct ether_addr
#include <stcp/socket.h>

#include <stdint.h>
#include <stddef.h>



enum {
    STCP_ETHER_ADDR_LEN = 6
};


namespace slank {



class ifaddr {
public:
    stcp_sa_family family;
    struct stcp_sockaddr raw;
    ifaddr(stcp_sa_family af, const stcp_sockaddr* addr) : family(af), raw(af)
    {
        switch (family) {
            case STCP_AF_LINK:
            {
                for (int i=0; i<6; i++) {
                    raw.sa_data[i] = addr->sa_data[i];
                }
                break;
            }
            case STCP_AF_INET:
            {
                stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&raw);
                const stcp_sockaddr_in* sin_addr= reinterpret_cast<const stcp_sockaddr_in*>(addr);
                sin->sin_addr = sin_addr->sin_addr;
                break;
            }
            case STCP_AF_INMASK:
            {
                stcp_sockaddr_in* sin = reinterpret_cast<stcp_sockaddr_in*>(&raw);
                const stcp_sockaddr_in* sin_addr = reinterpret_cast<const stcp_sockaddr_in*>(addr);
                sin->sin_addr = sin_addr->sin_addr;
                break;
            }
            default:
            {
                std::string errstr = "address family is not support ";
                errstr += std::to_string(family);
                throw exception(errstr.c_str());
                break;
            }
        }
    }
};




} /* namespace */
