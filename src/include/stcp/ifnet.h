

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <vector>

#include <stcp/config.h>
#include <stcp/ifaddr.h>
#include <stcp/protocol.h>
#include <stcp/socket.h>


namespace slank {
    



enum {
    BURST_SIZE = 64,
};


class ifnet : public proto_module {
private:
    uint8_t  port_id;
    uint16_t rx_ring_size;     /* rx ring size */
    uint16_t tx_ring_size;     /* tx ring size */
    uint16_t num_rx_rings;     /* num of rx_rings per port */
    uint16_t num_tx_rings;     /* num of tx_rings per port */

public:
    bool promiscuous_mode;
    std::vector<ifaddr> addrs;
    ifnet(uint8_t p) : 
        port_id(p),
        rx_ring_size(128),
        tx_ring_size(512),
        num_rx_rings(1  ),
        num_tx_rings(1  ),
        promiscuous_mode(true)
    { name = "PORT" + std::to_string(port_id); }

    void init();
    uint16_t io_rx();
    uint16_t io_tx(size_t num_request_to_send);
    size_t rx_size() { return rx.size(); }
    size_t tx_size() { return tx.size(); }
    void stat();

    void ioctl(uint64_t request, void* arg);
    void write(const void* buf, size_t bufsize);
    // size_t read(void* buf, size_t bufsize);

private:
    void ioctl_siocsifaddr(const stcp_ifreq* ifr);
    void ioctl_siocgifaddr(stcp_ifreq* ifr);
    void ioctl_siocsifhwaddr(const stcp_ifreq* ifr);
    void ioctl_siocgifhwaddr(stcp_ifreq* ifr);
    void ioctl_siocsifnetmask(const stcp_ifreq* ifr);
    void ioctl_siocgifnetmask(stcp_ifreq* ifr);
    void ioctl_siocpromisc(const uint64_t* val);
};




} /* namespace */
