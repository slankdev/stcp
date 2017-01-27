

#pragma once

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

#include <vector>
#include <string>

#include <dpdk/mempool.h>
#include <dpdk/cpu.h>
#include <dpdk/port.h>



class System;
class Command;

class Shell : public ssn_thread {
    std::vector<Command*> cmds;
    System* sys;
public:

    Shell(System* s);
    ~Shell();
    void add_cmd(Command* newcmd);
    void exe_cmd(const char* cmd_str);
    void operator()();
};


class System {
public:
    static size_t nb_rx_rings;
    static size_t nb_tx_rings;
    static size_t rx_ring_size;
    static size_t tx_ring_size;
    static size_t port_bulk_size;

	std::vector<dpdk::Cpu>  cpus;
	std::vector<dpdk::Port> ports;
	dpdk::Mempool           mp;
    Shell                   shell;
    bool                    cpuflags[RTE_CPUFLAG_NUMFLAGS];

	System(int argc, char** argv);
    ~System() { rte_eal_mp_wait_lcore(); }
    void halt();
	void launch();
};


