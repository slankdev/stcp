

#include <ssnlib_port.h>

namespace ssnlib {

size_t Port::id_next = 0;

size_t Port::nb_rx_rings    = 1;
size_t Port::nb_tx_rings    = 1;
size_t Port::rx_ring_size   = 128;
size_t Port::tx_ring_size   = 512;

} /* namespace ssnlib */
