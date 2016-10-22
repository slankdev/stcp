


#include <stcp/stcp.h>


namespace slank {


core& core::instance()
{
    static core s;
    return s;
}

void core::init(int argc, char** argv)
{
    dpdk.init(argc, argv);
    ip.init();
}

void core::ifs_proc()
{
    for (ifnet& dev : dpdk.devices) {
        uint16_t num_reqest_to_send = dev.tx_size();
        uint16_t num_tx = dev.io_tx(num_reqest_to_send);

        if (num_tx != num_reqest_to_send) {
            ; // TODO log to dmsg
        }

        uint16_t num_rx = dev.io_rx();
        if (unlikely(num_rx == 0)) continue;

        while (dev.rx_size() > 0) {
            mbuf* msg = dev.rx_pop();
            ether.rx_push(msg);
        }
    }
}

void core::run(bool endless)
{
    do {
        ifs_proc();
        ether.proc();
    } while (endless);
}

void core::stat_all()
{
    stat& s = stat::instance();
    s.clean();

    for (ifnet& dev : dpdk.devices) {
        dev.print_stat();
    }

    ether.print_stat();
    s.write("");
    arp.print_stat();
    s.write("");
    ip.print_stat();
    s.write("");
    icmp.print_stat();
    s.write("");
    udp.print_stat();

    s.flush();
}


} /* namespace */
