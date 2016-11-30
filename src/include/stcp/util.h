
#pragma once

#include <stcp/util.h>
#include <stddef.h>
#include <stcp/arch/dpdk/rte.h>

namespace slank {

struct stcp_ip_header;

inline uint32_t ntoh32(uint32_t n) noexcept {return rte::bswap32(n);}
inline uint16_t ntoh16(uint16_t n) noexcept {return rte::bswap16(n);}
inline uint32_t hton32(uint32_t n) noexcept {return rte::bswap32(n);}
inline uint16_t hton16(uint16_t n) noexcept {return rte::bswap16(n);}

inline uint16_t read_as_little_endian(const void* data)
{
    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
    return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
}

inline uint16_t checksum(const void* data, size_t len) noexcept
{
    uint32_t sum;
    const uint8_t* data_pointer = reinterpret_cast<const uint8_t*>(data);

    for (; len > 1; len-=2, data_pointer+=2) {
        uint16_t tmp = read_as_little_endian(data_pointer);
        sum += tmp;
    }

    if (len == 1) {
        uint8_t tmp = *data_pointer;
        sum += tmp;
    }

    uint16_t overflowd = sum >> 16;
    sum = sum & 0x0000ffff;
    sum = sum + overflowd;

    return ~sum;
}

inline uint16_t timediff_ms(uint64_t before, uint64_t after)
{
    uint64_t hz = rte::get_tsc_hz();
    return (after-before) * 1000 / hz;
}

inline uint16_t ipv4_udptcp_cksum(stcp_ip_header* ih, const void* th)
{
    return rte_ipv4_udptcp_cksum(
        reinterpret_cast<ipv4_hdr*>(ih), th);
}

inline uint64_t rand()
{
    return rte::rand();
}

inline void srand(uint64_t seedval)
{
    rte::srand(seedval);
}

inline uint64_t rdtsc()
{
    return rte_rdtsc();
}

inline uint64_t tsc_hz()
{
    return rte_get_tsc_hz();
}

inline void* malloc(const char* type, size_t size)
{
    return rte::malloc(type, size, 0);
}
inline void free(void *ptr)
{
    rte::free(ptr);
}

inline void *memcpy (void *dst, const void *src, size_t n)
{
    return rte::memcpy(dst, src, n);
}



} /* namespace slank */
