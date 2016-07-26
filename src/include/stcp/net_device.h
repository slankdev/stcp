



#pragma once
#include <stcp/rte.h>
#include <stcp/dpdk.h>
#include <slankdev.h>
#include <vector>
#include <string>




enum address_family {
    link,
    inet
};
class if_addr {
    public:
        enum address_family af;
        uint8_t data[16];

        if_addr(address_family a, const void* raw, size_t rawlen)
        {
            if (rawlen > sizeof(data))
                exit(-1);

            af = a;
            memcpy(data, raw, rawlen);
        }

    
};





class net_device {
    public:
        std::string name;
        std::vector<if_addr> addrs;

        net_device(std::string n) : name(n)
        {
            printf("[net_device:%s] init \n", name.c_str());
        }
        void set_hw_addr(struct ether_addr* addr)
        {
            if_addr ifaddr(link, addr, sizeof(struct ether_addr));
            addrs.push_back(ifaddr);

            printf("[net_device:%s] set address ", name.c_str());
            for (int i=0; i<6; i++) {
                printf("%02x", addr->addr_bytes[i]);
                if (i==5) printf("\n");
                else      printf(":");
            }
        }
};

