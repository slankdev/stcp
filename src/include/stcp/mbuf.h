
#include <stdint.h>
#include <stddef.h>
#include <stcp/rte.h>

namespace slank {
    

void  mbuf_pull(mbuf* msg, size_t len);
uint8_t* mbuf_push(mbuf* msg, size_t len);
void copy_to_mbuf(mbuf* mbuf, const void* buf, size_t bufsize);

} /* namespace */
