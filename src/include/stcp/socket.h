

#pragma once 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>




namespace slank {

struct stcp_in_addr stcp_inet_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4);
struct stcp_in_addr stcp_inet_addr(const char* fmt);
struct stcp_sockaddr stcp_inet_hwaddr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t o5, uint8_t o6);
char* p_sockaddr_to_str(const struct stcp_sockaddr* sa);
char* hw_sockaddr_to_str(const struct stcp_sockaddr* sa);


enum {
    STCP_IFNAMSIZ=16,
};

enum stcp_sa_family : uint16_t {
    STCP_AF_LINK = 0x1000,
    STCP_AF_INET,
    STCP_AF_ARP,
    STCP_AF_PACKET,
    STCP_AF_INMASK,
};





struct stcp_sockaddr {
	uint8_t         sa_len;       /* total length */
	stcp_sa_family  sa_fam;	      /* address family */
	uint8_t         sa_data[14];  /* actually longer; address value */

public:
    bool operator==(const stcp_sockaddr& rhs) const
    {
        return memcmp(this, &rhs, sizeof(stcp_sockaddr)) == 0;
    }
    bool operator!=(const stcp_sockaddr& rhs) const
    {
        return !(*this == rhs);
    }
    stcp_sockaddr& operator=(const stcp_sockaddr& rhs)
    {
        sa_len = rhs.sa_len;
        sa_fam = rhs.sa_fam;
        memcpy(sa_data, rhs.sa_data, sizeof(sa_data));
        return *this;
    }

    void inet_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4);
    void inet_hwaddr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t o5, uint8_t o6);
};

struct stcp_in_addr {
    uint8_t addr_bytes[4];

public:
    bool operator==(const stcp_in_addr& rhs) const 
    {
        for (int i=0; i<4; i++) {
            if (addr_bytes[i] != rhs.addr_bytes[i]) 
                return false;
        }
        return true;
    }
    bool operator!=(const stcp_in_addr& rhs) const
    {
        return !(*this == rhs);
    }
    stcp_in_addr& operator=(const stcp_in_addr& rhs)
    {
        for (int i=0; i<4; i++) {
            this->addr_bytes[i] = rhs.addr_bytes[i];
        }
        return *this;
    }
};

struct stcp_sockaddr_in {
	uint8_t	            sin_len;
	stcp_sa_family      sin_fam;
	uint16_t            sin_port;
	struct stcp_in_addr sin_addr;
	uint8_t	            sin_zero[8];

public:
    bool operator==(const stcp_sockaddr_in& rhs) const
    {
        bool r1 = (sin_len  == rhs.sin_len );
        bool r2 = (sin_fam  == rhs.sin_fam );
        bool r3 = (sin_port == rhs.sin_port);
        bool r4 = (sin_addr == rhs.sin_addr);
        return r1 && r2 && r3 && r4;
    }
    bool operator!=(const stcp_sockaddr_in& rhs) const
    {
        return !(*this==rhs);
    }
    stcp_sockaddr_in& operator=(const stcp_sockaddr_in& rhs)
    {
        sin_len  = rhs.sin_len ;
        sin_fam  = rhs.sin_fam ;
        sin_port = rhs.sin_port;
        sin_addr = rhs.sin_addr;
        return *this;
    }
};



// struct stcp_sockaddr_ll {
//     uint16_t sll_family;
//     uint16_t sll_protocol;
//     int      sll_ifindex;
//     uint16_t sll_hatype;
//     uint8_t  sll_pkttype;
//     uint8_t  sll_halen;
//     uint8_t  sll_addr[8];
// };



// struct stcp_sockaddr_dl {
// 	uint8_t	  sdl_len;	#<{(| Total length of sockaddr |)}>#
// 	uint8_t   sdl_family;	#<{(| AF_LINK |)}>#
// 	uint16_t  sdl_index;	#<{(| if != 0, system given index for interface |)}>#
// 	uint8_t	  sdl_type;	#<{(| interface type |)}>#
// 	uint8_t	  sdl_nlen;	#<{(| interface name length, no trailing 0 reqd. |)}>#
// 	uint8_t	  sdl_alen;	#<{(| link level address length |)}>#
// 	uint8_t	  sdl_slen;	#<{(| link layer selector length |)}>#
// 	char      sdl_data[46];	#<{(| minimum work area, can be larger;
// 			                 contains both if name and ll address |)}>#
// };


/* 
 * TODO 2016/09/13
 * If all names of member variables is ifr_** then 
 * compiler has error that 
 * "error: expected ‘;’ at end of member declaration"
 * but all names are if_** then compiler has not error,
 */
#if 0
struct stcp_ifreq {
    char ifr_name[IFNAMSIZ]; /* Interface name */
    union {
        struct sockaddr ifr_addr;
        struct sockaddr ifr_dstaddr;
        struct sockaddr ifr_broadaddr;
        struct sockaddr ifr_netmask;
        struct sockaddr ifr_hwaddr;
        short           ifr_flags;
        int             ifr_ifindex;
        int             ifr_metric;
        int             ifr_mtu;
        // struct ifmap    ifr_map;
        char            ifr_slave[IFNAMSIZ];
        char            ifr_newname[IFNAMSIZ];
        char            *ifr_data;
    };
};
#else
struct stcp_ifreq {
    char if_name[STCP_IFNAMSIZ]; /* Interface name */
    uint8_t if_index;
    union {
        struct stcp_sockaddr if_addr;
        struct stcp_sockaddr if_dstaddr;
        struct stcp_sockaddr if_broadaddr;
        struct stcp_sockaddr if_netmask;
        struct stcp_sockaddr if_hwaddr;
        short           if_flags;
        // int             if_ifindex;
        int             if_metric;
        int             if_mtu;
        char            if_slave[STCP_IFNAMSIZ];
        char            if_newname[STCP_IFNAMSIZ];
        char            *if_data;
    };
};
#endif



// class socket_impl {
// public:
//     virtual void open(stcp_sa_family domain);
//     virtual void close();
//     virtual void ioctl();
//     // virtual void write();
//     // virtual size_t read();
// };
//
// class socket {
// private:
//     socket_impl* sock;
// public:
//     socket(stcp_sa_family domain)
//     {
//     }
//
//     void 
// };



} /* namespace slankdev */
