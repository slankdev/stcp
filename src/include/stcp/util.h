
#pragma once

#include <stcp/util.h>
#include <stddef.h>
#include <stcp/util.h>
#include <stcp/rte.h>

namespace slank {

inline uint32_t ntoh32(uint32_t n) noexcept {return rte::bswap32(n);}
inline uint16_t ntoh16(uint16_t n) noexcept {return rte::bswap16(n);}
inline uint32_t hton32(uint32_t n) noexcept {return rte::bswap32(n);}
inline uint16_t hton16(uint16_t n) noexcept {return rte::bswap16(n);}

uint16_t checksum(const void* data, size_t len) noexcept;


} /* namespace slank */
