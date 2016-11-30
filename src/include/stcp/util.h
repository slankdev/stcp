
#pragma once

#include <stcp/util.h>
#include <stddef.h>
#include <stcp/util.h>
#include <stcp/arch/dpdk/rte.h>

namespace slank {

struct stcp_ip_header;

inline uint32_t ntoh32(uint32_t n) noexcept {return rte::bswap32(n);}
inline uint16_t ntoh16(uint16_t n) noexcept {return rte::bswap16(n);}
inline uint32_t hton32(uint32_t n) noexcept {return rte::bswap32(n);}
inline uint16_t hton16(uint16_t n) noexcept {return rte::bswap16(n);}

uint16_t checksum(const void* data, size_t len) noexcept;

uint16_t timediff_ms(uint64_t before, uint64_t after);
uint16_t ipv4_udptcp_cksum(stcp_ip_header* ih, const void* th);
uint64_t rand();
void srand(uint64_t seedval);
uint64_t rdtsc();
uint64_t tsc_hz();
void* malloc(const char* type, size_t size);
void free(void *ptr);
void *memcpy (void *dst, const void *src, size_t n);



} /* namespace slank */
