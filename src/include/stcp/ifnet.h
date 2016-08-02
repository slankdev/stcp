

#pragma once

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>

#include <string>
#include <iostream>
#include <sstream>
#include <exception>
#include <vector>

// #include <stcp/rte.h>
#include <stcp/config.h>
#include <stcp/ifaddr.h>
#include <stcp/protocol.h>






#define BURST_SIZE 32
class ifnet : public proto_module {
private:
    int port_id;
    uint16_t rx_ring_size;     /* rx ring size */
    uint16_t tx_ring_size;     /* tx ring size */
    uint16_t num_rx_rings;     /* num of rx_rings per port */
    uint16_t num_tx_rings;     /* num of tx_rings per port */

public:
    bool promiscuous_mode;
    // uint32_t rx_packets;
    // uint32_t tx_packets;

public:
    // pkt_queue rx;
    // pkt_queue tx;
    // std::string name;
    std::vector<ifaddr> addrs;

public:
    ifnet(int n) : 
        port_id(n),
        rx_ring_size(128),
        tx_ring_size(512),
        num_rx_rings(1  ),
        num_tx_rings(1  ),
        promiscuous_mode(true)
        // rx_packets(0), 
        // tx_packets(0)
    {
        name = "PORT" + std::to_string(n);
    }
    void init();
    uint16_t io_rx();
    uint16_t io_tx(size_t num_request_to_send);
    size_t rx_size() { return rx.size(); }
    size_t tx_size() { return tx.size(); }
    void stat();
};



