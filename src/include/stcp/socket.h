

#pragma once 



namespace slank {


enum {
    STCP_IFNAMSIZ=16,
};

enum stcp_sa_family {
    STCP_AF_LINK=0,
    STCP_AF_INET=2,
    STCP_AF_ARP,
};

struct stcp_sockaddr {
	uint8_t         sa_len;		/* total length */
	stcp_sa_family  sa_fam;	/* address family */
	char            sa_data[14];	/* actually longer; address value */
};

struct stcp_in_addr {
    uint8_t addr_bytes[4];

    bool operator==(const struct stcp_in_addr& rhs) 
    {
        for (int i=0; i<4; i++) {
            if (addr_bytes[i] != rhs.addr_bytes[i]) return false;
        }
        return true;
    }
};

struct stcp_sockaddr_in {
	uint8_t	            sin_len;
	stcp_sa_family      sin_fam;
	uint16_t            sin_port;
	struct stcp_in_addr sin_addr;
	char	            sin_zero[8];
};


struct stcp_arpreq {
    struct stcp_sockaddr arp_pa;		/* Protocol address.  */
    struct stcp_sockaddr arp_ha;		/* Hardware address.  */
    uint8_t              arp_ifindex;
};

// struct stcp_sockaddr_inarp {
//     uint8_t             sin_len;
//     uint8_t             sin_family;
//     uint16_t            sin_port;
//     struct stcp_in_addr sin_addr;
//     struct stcp_in_addr sin_srcaddr;
//     uint16_t            sin_tos;
//     uint16_t            sin_other;
// };


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



// class socket {
// private:
//     void socket_inet(int type, int protocol);
//     void socket_link(int type, int protocol);
//
// public:
//     socket(stcp_sa_family domain, int type, int protocol) {
//         switch (domain) {
//             case STCP_AF_INET:
//             {
//                 socket_inet(type, protocol);
//                 break;
//             }
//             case STCP_AF_LINK:
//             {
//                 socket_link(type, protocol);
//                 break;
//                 break;
//             }
//             default:
//             {
//                 throw slankdev::exception("not supported");
//                 break;
//             }
//         }
//     }
//
// };



} /* namespace slankdev */
