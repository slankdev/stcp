

#include <stdint.h>
#include <stddef.h>
#include <stcp/dpdk.h>


namespace slank {

uint32_t dpdk_core::ipv4_mtu_default = ETHER_MTU;
// uint32_t dpdk_core::ipv4_mtu_default = 1300;
uint32_t dpdk_core::num_mbufs = 8192;
uint32_t dpdk_core::mbuf_cache_size = 250;
std::string dpdk_core::mp_name = "STCP";

} /* namespace slank */
