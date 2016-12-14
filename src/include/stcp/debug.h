
#pragma once

#include <stddef.h>

namespace stcp {



#if ST_RUNLEVEL==RUNLEV_SPEED

# define DPRINT(...) printf(...)
# define stcp_printf(...)

#elif ST_RUNLEVEL==RUNLEV_DEBUG

# define DPRINT(...) \
    core::stcp_stddbg.fprintf("%-15s:%4d: ", __FILE__, __LINE__); \
    core::stcp_stddbg.fprintf(__VA_ARGS__); \
    core::stcp_stddbg.fflush();
# define stcp_printf(...) \
    core::stcp_stdout.fprintf(__VA_ARGS__); \
    core::stcp_stdout.fflush();

#else
# error "unknown runlevel"
#endif





} /* namespace stcp */
