



#include <stcp/stcp.h>
#include <stcp/rte.h>

#include <stdlib.h>
#include <unistd.h>




void stcp::init(int argc, char** argv)
{
    log& log = log::instance();
    log.write(INFO, "init...");

    dpdk& dpdk = dpdk::instance();
    dpdk.init(argc, argv);
    arp.init();
}

void stcp::run()
{
    log& log = log::instance();
    log.write(INFO, "start running");

    while (true) {
        ifs_proc();
        arp.proc();
        // ip.proc();
    }
}


struct ether_header {
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t type;
};

uint16_t get_ether_type(struct rte_mbuf* msg)
{
    struct ether_header* eh;
    eh = rte_pktmbuf_mtod(msg, struct ether_header*);
    return rte_bswap16(eh->type);
}



void stcp::ifs_proc()
{
    dpdk& dpdk = dpdk::instance();
    log& log = log::instance();

    for (net_device& dev : dpdk.devices) {
        uint16_t num_rx = dev.io_rx();
        if (unlikely(num_rx == 0)) return;

        uint16_t num_reqest_to_send = dev.tx_size();
        uint16_t num_tx = dev.io_tx(num_reqest_to_send);
        if (num_tx != num_reqest_to_send)
            fprintf(stderr, "some packet droped \n");



        while (dev.rx_size() > 0) {
            struct rte_mbuf* msg = dev.rx_pop();
            uint16_t etype = get_ether_type(msg);
            switch (etype) {
                case 0x0800:
                {
                    printf("ip \n");
                    ip.rx_push(msg);
                    break;
                }
                case 0x0806:
                {
                    printf("arp \n");
                    arp.rx_push(msg);
                    break;
                }
                default:
                {
                    log.write(WARN, "unknown ether type 0x%04x", etype);
                    rte::pktmbuf_free(msg);
                    break;
                }
            }
            stat_all();
        }
    }
}

void stcp::stat_all()
{
    dpdk& dpdk = dpdk::instance();
    clear_screen();

    for (net_device& dev : dpdk.devices) {
        dev.stat();
    }

    arp.stat();
    ip.stat();
}








