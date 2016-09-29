

#pragma once
#include <stdint.h>
#include <stddef.h>


namespace slank {

void set_netmask(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4);
void set_ip_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4);
void set_hw_addr(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t o5, uint8_t o6);
void set_default_gw(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint8_t port);
void add_arp_record(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4,
        uint8_t ho1, uint8_t ho2, uint8_t ho3, uint8_t ho4, uint8_t ho5, uint8_t ho6);


} /* namespace slank */

