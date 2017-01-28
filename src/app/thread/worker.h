
#pragma once


int thread_wk(void* arg)
{
    dpdk::System* sys = reinterpret_cast<dpdk::System*>(arg);
    const uint8_t nb_ports = sys->ports.size();
	for (;;) {
		for (uint8_t pid=0; pid<nb_ports; pid++) {
            dpdk::Port& in_port  = sys->ports[pid];
            dpdk::Port& out_port = sys->ports[pid^1];

            const size_t burst_size = 32;
            rte_mbuf* pkts[burst_size];
            bool ret = in_port.rxq[0].pop_bulk(pkts, burst_size);
            if (ret) out_port.txq[0].push_bulk(pkts, burst_size);
	    }
	}
    return 0;
}

