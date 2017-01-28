
#pragma once


class ssnt_txrxwk : public ssnlib::ssn_thread {
    ssnlib::System* sys;
public:
    ssnt_txrxwk(ssnlib::System* s) : sys(s) {}
    void operator()()
    {
        const uint8_t nb_ports = sys->ports.size();
        for (;;) {
#if 0
            for (uint8_t pid = 0; pid < nb_ports; pid++) {
                uint8_t nb_rxq = sys->ports[pid].rxq.size();
                uint8_t nb_txq = sys->ports[pid].txq.size();
                assert(nb_txq == nb_rxq);

                for (uint8_t qid=0; qid<nb_rxq; qid++) {
                    ssnlib::Port& in_port  = sys->ports[pid];
                    ssnlib::Port& out_port = sys->ports[pid^1];

                    in_port.rx_burst_bulk(qid);

                    const size_t burst_size = 32;
                    rte_mbuf* pkts[burst_size];
                    bool ret = in_port.rxq[qid].pop_bulk(pkts, burst_size);
                    if (ret) out_port.txq[qid].push_bulk(pkts, burst_size);

                    out_port.tx_burst_bulk(qid);
                }
            }
#else
            for (uint8_t pid = 0; pid < nb_ports; pid++) {
                uint8_t nb_rxq = sys->ports[pid].rxq.size();
                uint8_t nb_txq = sys->ports[pid].txq.size();
                assert(nb_txq == nb_rxq);

                for (uint8_t qid=0; qid<nb_rxq; qid++) {
                    ssnlib::Port& in_port  = sys->ports[pid];
                    ssnlib::Port& out_port = sys->ports[pid^1];

                    in_port.rxq[qid].burst_bulk();

                    const size_t burst_size = 32;
                    rte_mbuf* pkts[burst_size];
                    bool ret = in_port.rxq[qid].pop_bulk(pkts, burst_size);
                    if (ret) out_port.txq[qid].push_bulk(pkts, burst_size);

                    out_port.txq[qid].burst_bulk();
                }

            } // for port
#endif
        }

    }
};




class ssnt_rx : public ssnlib::ssn_thread {
    ssnlib::System* sys;
public:
    ssnt_rx(ssnlib::System* s) : sys(s) {}
    void operator()()
    {
        const uint8_t nb_ports = sys->ports.size();
        for (;;) {
            for (uint8_t pid = 0; pid < nb_ports; pid++) {
                uint8_t nb_rxq = sys->ports[pid].rxq.size();
                for (uint8_t qid=0; qid<nb_rxq; qid++) {
                    sys->ports[pid].rxq[qid].burst_bulk();
                }
            }
        }
    }
};



class ssnt_tx : public ssnlib::ssn_thread {
    ssnlib::System* sys;
public:
    ssnt_tx(ssnlib::System* s) : sys(s) {}
    void operator()()
    {
        const uint8_t nb_ports = sys->ports.size();
        for (;;) {
            for (uint8_t pid = 0; pid < nb_ports; pid++) {
                uint8_t nb_txq = sys->ports[pid].txq.size();
                for (uint8_t qid=0; qid<nb_txq; qid++) {
                    sys->ports[pid].txq[qid].burst_bulk();
                }
            }
        }
    }
};


class ssnt_wk : public ssnlib::ssn_thread {
    ssnlib::System* sys;
public:
    ssnt_wk(ssnlib::System* s) : sys(s) {}
    void operator()()
    {
        const uint8_t nb_ports = sys->ports.size();
        for (;;) {
            for (uint8_t pid = 0; pid < nb_ports; pid++) {
                uint8_t nb_rxq = sys->ports[pid].rxq.size();
                uint8_t nb_txq = sys->ports[pid].txq.size();
                assert(nb_rxq == nb_txq);

                for (uint8_t qid=0; qid<nb_rxq; qid++) {
                    ssnlib::Port& in_port  = sys->ports[pid];
                    ssnlib::Port& out_port = sys->ports[pid^1];

                    const size_t burst_size = 32;
                    rte_mbuf* pkts[burst_size];
                    bool ret = in_port.rxq[qid].pop_bulk(pkts, burst_size);
                    if (ret) out_port.txq[qid].push_bulk(pkts, burst_size);
                }
            }
        }
    }
};


