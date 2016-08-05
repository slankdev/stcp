

#pragma once


#include <stdint.h>
#include <stddef.h>
#include <rte_mbuf.h> // for struct ether_header

enum vars {
    ifnamesiz=16
};
// #define IFNAMSIZ 16



namespace slank {

struct ip_addr {
    uint8_t addr_bytes[4];

    bool operator==(const struct ip_addr& rhs) 
    {
        for (int i=0; i<4; i++) {
            if (addr_bytes[i] != rhs.addr_bytes[i]) return false;
        }
        return true;
    }
};

struct ether_header {
    struct ether_addr dst;
    struct ether_addr src;
    uint16_t type;
};

struct arphdr {
    uint16_t          hwtype;
    uint16_t          ptype;
    uint8_t           hwlen;
    uint8_t           plen;
    uint16_t          operation;
    struct ether_addr hwsrc;
    struct ip_addr    psrc;
    struct ether_addr hwdst;
    struct ip_addr    pdst;
};


enum sa_family {
    af_link=0,
    af_inet=2,
};

struct sockaddr {
	uint8_t      sa_len;		/* total length */
	sa_family    sa_fam;	/* address family */
	char         sa_data[14];	/* actually longer; address value */
};


struct sockaddr_in {
	uint8_t	       sin_len;
	sa_family	   sin_fam;
	uint16_t       sin_port;
	struct ip_addr sin_addr;
	char	       sin_zero[8];
};


// struct sockaddr_dl {
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


// struct ifreq {
//     char ifr_name[IFNAMSIZ]; #<{(| Interface name |)}>#
//     union {
//         struct sockaddr ifr_addr;
//         struct sockaddr ifr_dstaddr;
//         struct sockaddr ifr_broadaddr;
//         struct sockaddr ifr_netmask;
//         struct sockaddr ifr_hwaddr;
//         short           ifr_flags;
//         int             ifr_ifindex;
//         int             ifr_metric;
//         int             ifr_mtu;
//         // struct ifmap    ifr_map;
//         char            ifr_slave[IFNAMSIZ];
//         char            ifr_newname[IFNAMSIZ];
//         char            *ifr_data;
//     };
// };


} /* namespace */
