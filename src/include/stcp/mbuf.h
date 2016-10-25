
#include <stdint.h>
#include <stddef.h>
#include <stcp/rte.h>

namespace slank {

void  mbuf_pull(mbuf* msg, size_t len);
uint8_t* mbuf_push(mbuf* msg, size_t len);

} /* namespace */
