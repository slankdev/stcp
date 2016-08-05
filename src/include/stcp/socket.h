

#pragma once

#include <stdint.h>
#include <stddef.h>

#define IFNAMSIZ 16

namespace slank {
    


struct sockaddr {
	uint8_t      sa_len;		/* total length */
	sa_family_t  sa_family;	/* address family */
	char         sa_data[14];	/* actually longer; address value */
};


struct sockaddr_in {
	uint8_t	       sin_len;
	sa_family_t	   sin_family;
	in_port_t      sin_port;
	struct in_addr sin_addr;
	char	       sin_zero[8];
};


struct sockaddr_dl {
	uint8_t	  sdl_len;	/* Total length of sockaddr */
	uint8_t   sdl_family;	/* AF_LINK */
	uint16_t  sdl_index;	/* if != 0, system given index for interface */
	uint8_t	  sdl_type;	/* interface type */
	uint8_t	  sdl_nlen;	/* interface name length, no trailing 0 reqd. */
	uint8_t	  sdl_alen;	/* link level address length */
	uint8_t	  sdl_slen;	/* link layer selector length */
	char      sdl_data[46];	/* minimum work area, can be larger;
			                 contains both if name and ll address */
};


struct ifreq {
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
        struct ifmap    ifr_map;
        char            ifr_slave[IFNAMSIZ];
        char            ifr_newname[IFNAMSIZ];
        char            *ifr_data;
    };
};


} /* namespace */
