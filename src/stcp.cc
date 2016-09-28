


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
    arp.init();
    ip.init();
}

void core::ifs_proc()
{
    for (ifnet& dev : dpdk.devices) {
        uint16_t num_reqest_to_send = dev.tx_size();
        uint16_t num_tx = dev.io_tx(num_reqest_to_send);
        if (num_tx != num_reqest_to_send)
            throw slankdev::exception("some packet droped");

        uint16_t num_rx = dev.io_rx();
        if (unlikely(num_rx == 0)) continue;

        modules_updated = true;
        while (dev.rx_size() > 0) {
            mbuf* msg = dev.rx_pop();
            ether.rx_push(msg);
        }
    }
}

void core::run()
{
    stat_all();
    while (true) {
        modules_updated = false;

        ifs_proc();
        ether.proc();
        arp.proc();
        ip.proc();


        if (modules_updated)
            stat_all();
    }
}

void core::stat_all()
{
    clear_screen();

    for (ifnet& dev : dpdk.devices) {
        dev.stat();
    }

    ether.stat();
    arp.stat();
    ip.stat();
}


} /* namespace */
