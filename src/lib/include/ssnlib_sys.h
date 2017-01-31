

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
#include <ssnlib_misc.h>

#include <slankdev/exception.h>


namespace ssnlib {



template <class PORT=Port<>, class CPU=Cpu>
class System {
public:
	std::vector<CPU>  cpus;
	std::vector<PORT> ports;
	ssnlib::Mempool           mp;
    bool                    cpuflags[RTE_CPUFLAG_NUMFLAGS];

	System_d(int argc, char** argv)
    {
        /*
         * Boot DPDK System.
         */
        kernel_log(SYSTEM, "[+] Booting ...\n");
        ssnlib::print_message();

        /*
         * DPDK init
         */
        int ret = rte_eal_init(argc, argv);
        if (ret < 0) {
            throw slankdev::exception("rte_eal_init");
        }

        kernel_log(SYSTEM, "configure \n");
        cpus.resize(rte_lcore_count());
        ports.resize(rte_eth_dev_count());

        for (size_t i=0; i<RTE_CPUFLAG_NUMFLAGS; i++)
            cpuflags[i] = rte_cpu_get_flag_name(rte_cpu_flag_t(i));

        kernel_log(SYSTEM, "[+] DPDK boot Done! \n");
    }


    virtual ~System_d() { rte_eal_mp_wait_lcore(); }
    void halt()
    {
        kernel_log(SYSTEM, "[+] System Halt ...\n");
        rte_exit(0, "Bye...\n");
    }


	void wait_all()
    {
        sleep(1);
        rte_eal_mp_wait_lcore();
    }
};


class System : public System_d<> {
public:
    System(int argc, char** argv) : System_d<>(argc, argv) {}
};


} /* namespace ssnlib */


