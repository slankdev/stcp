

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <rte_mbuf.h> // for struct ether_header






namespace slank {

enum vars {
    STCP_IFNAMSIZ=16,
};

struct stcp_ip_addr {
    uint8_t addr_bytes[4];

    bool operator==(const struct stcp_ip_addr& rhs) 
    {
        for (int i=0; i<4; i++) {
            if (addr_bytes[i] != rhs.addr_bytes[i]) return false;
        }
        return true;
    }
};

struct stcp_ether_header {
    struct ether_addr dst;
    struct ether_addr src;
    uint16_t type;
};

struct stcp_arphdr {
    uint16_t            hwtype;
    uint16_t            ptype;
    uint8_t             hwlen;
    uint8_t             plen;
    uint16_t            operation;
    struct ether_addr   hwsrc;
    struct stcp_ip_addr psrc;
    struct ether_addr   hwdst;
    struct stcp_ip_addr pdst;
};


enum stcp_sa_family {
    STCP_AF_LINK=0,
    STCP_AF_INET=2,
};

struct stcp_sockaddr {
	uint8_t         sa_len;		/* total length */
	stcp_sa_family  sa_fam;	/* address family */
	char            sa_data[14];	/* actually longer; address value */
};


struct stcp_sockaddr_in {
	uint8_t	            sin_len;
	stcp_sa_family      sin_fam;
	uint16_t            sin_port;
	struct stcp_ip_addr sin_addr;
	char	            sin_zero[8];
};

struct stcp_sockaddr_dl {
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
    union {
        struct stcp_sockaddr if_addr;
        struct stcp_sockaddr if_dstaddr;
        struct stcp_sockaddr if_broadaddr;
        struct stcp_sockaddr if_netmask;
        struct stcp_sockaddr if_hwaddr;
        short           if_flags;
        int             if_ifindex;
        int             if_metric;
        int             if_mtu;
        // struct ifmap    ifr_map;
        char            if_slave[STCP_IFNAMSIZ];
        char            if_newname[STCP_IFNAMSIZ];
        char            *if_data;
    };
};
#endif



} /* namespace */
