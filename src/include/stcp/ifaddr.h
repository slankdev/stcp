
#pragma once

#include <stcp/config.h>
#include <stcp/rte.h> // struct ether_addr
#include <stcp/types.h>
#include <stcp/socket.h>

#include <stdint.h>
#include <stddef.h>



enum {
    STCP_ETHER_ADDR_LEN = 6
};


namespace slank {
    


const char* af2str(stcp_sa_family af);

struct stcp_in_addr stcp_inet_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4);
struct stcp_in_addr stcp_inet_addr(const char* fmt);


class ifaddr {
public:
    stcp_sa_family family;
    struct stcp_sockaddr raw;
    // ifaddr(stcp_sa_family af) : family(af) {}
    ifaddr(stcp_sa_family af, const stcp_sockaddr* addr) : family(af)
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
            default:
            {
                throw slankdev::exception("address family is not support");
                break;
            }
        }
    }
};




} /* namespace */
