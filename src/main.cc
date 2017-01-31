

#include <stdio.h>
#include <ssnlib_sys.h>
#include <ssnlib_shell.h>


#include <ssnlib_thread.h>
#include <slankdev/system.h>
class ssnt_txrxwk : public ssnlib::ssn_thread {
    ssnlib::System* sys;
    bool running;
public:
    ssnt_txrxwk(ssnlib::System* s) : sys(s), running(false) {}
    void operator()()
    {
        const uint8_t nb_ports = sys->ports.size();
        running = true;
        while (running) {
            for (uint8_t pid = 0; pid < nb_ports; pid++) {
                uint8_t nb_rxq = sys->ports[pid].rxq.size();
                uint8_t nb_txq = sys->ports[pid].txq.size();
                assert(nb_txq == nb_rxq);

                for (uint8_t qid=0; qid<nb_rxq; qid++) {
                    auto& in_port  = sys->ports[pid];
                    auto& out_port = sys->ports[pid^1];

                    in_port.rxq[qid].burst_bulk();

                    const size_t burst_size = 32;
                    rte_mbuf* pkts[burst_size];
                    bool ret = in_port.rxq[qid].pop_bulk(pkts, burst_size);
                    if (ret) out_port.txq[qid].push_bulk(pkts, burst_size);

                    out_port.txq[qid].burst_bulk();
                }
            }
        }

    }
    bool kill() { running=false; return true; }
};
class ssnt_rx : public ssnlib::ssn_thread {
    ssnlib::System* sys;
    bool running;
public:
    ssnt_rx(ssnlib::System* s) : sys(s), running(false) {}
    void operator()()
    {
        const uint8_t nb_ports = sys->ports.size();
        running = true;
        while (running) {
            for (uint8_t pid = 0; pid < nb_ports; pid++) {
                uint8_t nb_rxq = sys->ports[pid].rxq.size();
                for (uint8_t qid=0; qid<nb_rxq; qid++) {
                    sys->ports[pid].rxq[qid].burst_bulk();
                }
            }
        }
    }
    bool kill() { running=false; return true; }
};
class ssnt_tx : public ssnlib::ssn_thread {
    ssnlib::System* sys;
    bool running;
public:
    ssnt_tx(ssnlib::System* s) : sys(s), running(false) {}
    void operator()()
    {
        const uint8_t nb_ports = sys->ports.size();
        running = true;
        while (running) {
            for (uint8_t pid = 0; pid < nb_ports; pid++) {
                uint8_t nb_txq = sys->ports[pid].txq.size();
                for (uint8_t qid=0; qid<nb_txq; qid++) {
                    sys->ports[pid].txq[qid].burst_bulk();
                }
            }
        }
    }
    bool kill() { running=false; return true; }
};
class ssnt_wk : public ssnlib::ssn_thread {
    ssnlib::System* sys;
    bool running;
    size_t nb_delay_clk;
public:
    ssnt_wk(ssnlib::System* s) : sys(s), running(false), nb_delay_clk(0) {}
    ssnt_wk(ssnlib::System* s, size_t d) : sys(s), running(false), nb_delay_clk(d) {}
    void operator()()
    {
        const uint8_t nb_ports = sys->ports.size();
        running = true;
        while (running) {
            for (uint8_t pid = 0; pid < nb_ports; pid++) {
                uint8_t nb_rxq = sys->ports[pid].rxq.size();
                uint8_t nb_txq = sys->ports[pid].txq.size();
                assert(nb_rxq == nb_txq);

                for (uint8_t qid=0; qid<nb_rxq; qid++) {
                    auto& in_port  = sys->ports[pid];
                    auto& out_port = sys->ports[pid^1];

                    const size_t burst_size = 32;
                    rte_mbuf* pkts[burst_size];
                    bool ret = in_port.rxq[qid].pop_bulk(pkts, burst_size);
                    if (ret) {
                        slankdev::delay_clk(nb_delay_clk);
                        out_port.txq[qid].push_bulk(pkts, burst_size);
                    }
                }
            }
        }
    }
    bool kill() { running=false; return true; }
};

#include <ssnlib_sys.h>
#include <ssnlib_cmd.h>
class Cmd_test : public ssnlib::Command {
    ssnlib::System* sys;
    ssnlib::Shell*  shell;
public:
    Cmd_test(ssnlib::System* s, ssnlib::Shell* sh)
        : sys(s), shell(sh) { name = "test"; }
    void operator()(const std::vector<std::string>& args)
    {
        UNUSED(args);
        for (;;) {
            slankdev::clear_screen();
            shell->exe_cmd("show port");
            usleep(50000);
        }
    }
};
class Cmd_run : public ssnlib::Command {
    ssnlib::System* sys;
    ssnlib::Shell*  shell;
public:
    Cmd_run(ssnlib::System* s, ssnlib::Shell* sh)
        : sys(s), shell(sh) { name = "run"; }
    void operator()(const std::vector<std::string>& args)
    {
        UNUSED(args);
        shell->exe_cmd("thread launch 2");
        shell->exe_cmd("thread launch 3");
        shell->exe_cmd("thread launch 4");
    }
};




int main(int argc, char** argv)
{
    using namespace ssnlib;

    Port<>::nb_rx_rings    = 2;
    Port<>::nb_tx_rings    = 2;
    Port<>::rx_ring_size   = 128;
    Port<>::tx_ring_size   = 512;

    System sys(argc, argv);
    if (sys.ports.size()%2 != 0) return -1;

    Shell shell(&sys);
    shell.add_cmd(new Cmd_test    (&sys, &shell));
    shell.add_cmd(new Cmd_run     (&sys, &shell));

#if 0
    ssnt_txrxwk txrxwk(&sys);
    sys.cpus[1].thread = &shell;
    sys.cpus[2].thread = &txrxwk;
#else
    ssnt_rx rx(&sys);
    ssnt_tx tx(&sys);
    ssnt_wk wk(&sys, 40000);
    sys.cpus[1].thread = &shell;
    sys.cpus[2].thread = &rx;
    sys.cpus[3].thread = &tx;
    sys.cpus[4].thread = &wk;
    sys.cpus[5].thread = &wk;
    sys.cpus[6].thread = &wk;
    sys.cpus[8].thread = &wk;
    sys.cpus[9].thread = &wk;
    sys.cpus[10].thread = &wk;
    sys.cpus[11].thread = &wk;
#endif

    sys.cpus[1].launch();
    sys.wait_all();
}

