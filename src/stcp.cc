


#include <stdlib.h>

#include <stcp/stcp.h>
#include <stcp/rte.h>
#include <stcp/arp.h>
#include <stcp/ethernet.h>
#include <stcp/mbuf.h>


using namespace slank;


void stcp::user_setting()
{
    struct ip_addr a;
    a.addr_bytes[0] = 192;
    a.addr_bytes[1] = 168;
    a.addr_bytes[2] = 222;
    a.addr_bytes[3] = 254;

    struct sockaddr_in sin;
    sin.sin_fam = af_inet;
    sin.sin_addr = a;

    dpdk& dpdk = dpdk::instance();
    dpdk.devices[0].ioctl(siocsifaddr, &sin);
}



static uint16_t get_ether_type(struct rte_mbuf* msg)
{
    struct ether_header* eh;
    eh = rte_pktmbuf_mtod(msg, struct ether_header*);
    return rte_bswap16(eh->type);
}


void stcp::ifs_proc()
{
    dpdk& dpdk = dpdk::instance();
    log& log = log::instance();

    for (ifnet& dev : dpdk.devices) {
        uint16_t num_rx = dev.io_rx();
        if (unlikely(num_rx == 0)) continue;

        modules_updated = true;
        uint16_t num_reqest_to_send = dev.tx_size();
        uint16_t num_tx = dev.io_tx(num_reqest_to_send);
        if (num_tx != num_reqest_to_send)
            fprintf(stderr, "some packet droped \n");

        while (dev.rx_size() > 0) {
            struct rte_mbuf* msg = dev.rx_pop();
            uint16_t etype = get_ether_type(msg);
            mbuf_pull(msg, sizeof(struct ether_header));

            switch (etype) {
                case 0x0800:
                {
                    log.write(INFO, "recv ip packet");
                    ip.rx_push(msg);
                    break;
                }
                case 0x0806:
                {
                    log.write(INFO, "recv arp packet");
                    arp.rx_push(msg);
                    break;
                }
                default:
                {
                    log.write(WARN, "unknown ether type 0x%04x", etype);
                    dev.drop(msg);
                    break;
                }
            }
        }
    }
}




