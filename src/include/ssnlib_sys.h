

#pragma once

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

#include <vector>
#include <string>

#include <ssnlib_thread.h>
#include <ssnlib_cmd.h>
#include <ssnlib_mempool.h>
#include <ssnlib_cpu.h>
#include <ssnlib_port.h>



namespace ssnlib {


class System {
public:
    static size_t nb_rx_rings;
    static size_t nb_tx_rings;
    static size_t rx_ring_size;
    static size_t tx_ring_size;
    static size_t port_bulk_size;

	std::vector<ssnlib::Cpu>  cpus;
	std::vector<ssnlib::Port> ports;
	ssnlib::Mempool           mp;
    bool                    cpuflags[RTE_CPUFLAG_NUMFLAGS];

	System(int argc, char** argv);
    ~System() { rte_eal_mp_wait_lcore(); }
    void halt();
	void launch();
};


} /* namespace ssnlib */


