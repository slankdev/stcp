


#include <stcp/debug.h>
#include <stcp/stcp.h>
#include <stddef.h>

namespace stcp {


template <class... Args>
void DPRINT(const char* format, Args... args)
{
    core::stcp_stddbg.fprintf("%-15s:%4d: ", __FILE__, __LINE__); \
    core::stcp_stddbg.fprintf(format, args...); \
    core::stcp_stddbg.fflush(); \
}



} /* namespace stcp */
