
#pragma once


int thread_txrx_AP(void* arg)
{
    dpdk::System* sys = reinterpret_cast<dpdk::System*>(arg);

    const uint8_t nb_ports = sys->ports.size();
	for (;;) {
        for (uint8_t pid = 0; pid < nb_ports; pid++) {
            sys->ports[pid].rx_burst();
            sys->ports[pid].tx_burst();
	    }
	}
    return 0;
}
int thread_tx_AP(void* arg)
{
    dpdk::System* sys = reinterpret_cast<dpdk::System*>(arg);
    const uint8_t nb_ports = sys->ports.size();
	for (;;) {
        for (uint8_t pid = 0; pid < nb_ports; pid++) {
            sys->ports[pid].tx_burst();
	    }
	}
}
int thread_rx_AP(void* arg)
{
    dpdk::System* sys = reinterpret_cast<dpdk::System*>(arg);
    const uint8_t nb_ports = sys->ports.size();
	for (;;) {
        for (uint8_t pid = 0; pid < nb_ports; pid++) {
            sys->ports[pid].rx_burst();
	    }
	}
    return 0;
}


struct thread_txrx_arg {
    dpdk::System* sys;
    size_t port_id;
};
int thread_txrx(void* _arg)
{
    thread_txrx_arg* arg = reinterpret_cast<thread_txrx_arg*>(_arg);
    dpdk::System* sys = arg->sys;
    size_t        pid = arg->port_id;

	for (;;) {
        sys->ports[pid].rx_burst();
        sys->ports[pid].tx_burst();
	}
    return 0;
}


