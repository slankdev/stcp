
#pragma once



class ssnt_txrxwk : public ssn_thread {
    System* sys;
public:
    ssnt_txrxwk(System* s) : sys(s) {}
    void operator()()
    {
        const uint8_t nb_ports = sys->ports.size();
        for (;;) {
            for (uint8_t pid = 0; pid < nb_ports; pid++) {
                dpdk::Port& in_port  = sys->ports[pid];
                dpdk::Port& out_port = sys->ports[pid^1];

                in_port.rx_burst();

                const size_t burst_size = 32;
                rte_mbuf* pkts[burst_size];
                bool ret = in_port.rxq[0].pop_bulk(pkts, burst_size);
                if (ret) out_port.txq[0].push_bulk(pkts, burst_size);

                out_port.tx_burst();
            }
        }
    }
};

