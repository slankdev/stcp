
#pragma once

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>

#include <string>
#include <iostream>
#include <sstream>
#include <exception>
#include <vector>

#include <stcp/rte.h>
#include <stcp/config.h>

#define ETHER_ADDR_LEN 6

#define AF_LINK 0
#define AF_INET 2

typedef uint8_t af_t;


class ifaddr {
    public:
        af_t    family;
        struct {
            union {
                uint8_t data[16];
                struct ether_addr link;
                uint8_t in[4];
            };
        } raw;

    ifaddr(af_t af) : family(af) {}
    void init(const void* d, size_t l);
};




