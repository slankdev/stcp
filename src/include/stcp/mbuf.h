
#include <stdint.h>
#include <stddef.h>
#include <stcp/rte.h>


void  mbuf_pull(struct rte_mbuf* msg, size_t len);
uint8_t* mbuf_push(struct rte_mbuf* msg, size_t len);
