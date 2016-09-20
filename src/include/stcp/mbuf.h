
#include <stdint.h>
#include <stddef.h>
#include <stcp/rte.h>

namespace slank {
    

void  mbuf_pull(struct rte_mbuf* msg, size_t len);
uint8_t* mbuf_push(struct rte_mbuf* msg, size_t len);
void copy_to_mbuf(struct rte_mbuf* mbuf, const void* buf, size_t bufsize);

} /* namespace */
