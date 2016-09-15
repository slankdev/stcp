

#include <stdint.h>
#include <stddef.h>
#include <stcp/dpdk.h>


namespace slank {

uint32_t dpdk::num_mbufs = 8192;
uint32_t dpdk::mbuf_cache_size = 250;
std::string dpdk::mp_name = "STCP";

} /* namespace slank */
