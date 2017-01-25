

#pragma once

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

#include <vector>
#include <string>

#include <slankdev/exception.h>
#include <susanoo_log.h>
#include <susanoo_misc.h>

#include <dpdk/mempool.h>
#include <dpdk/cpu.h>
#include <dpdk/port.h>


/* for susanoo_shell */
#include <stdio.h>
#include <stdlib.h>
#include <susanoo_shell.h>



class System {
    class Shell : public ssn_thread {
        System* sys;
        sush sush0;
    public:
        Shell(System* s) : sys(s) {}
        void add_cmd(Command* t)
        {
            sush0.add_cmd(t);
        }
        void operator()()
        {
            printf("\n\n");
            sush0.main_loop();
            return;
        }
    };

public:
    static size_t rx_ring_size;
    static size_t tx_ring_size;
    static size_t port_bulk_size;

	std::vector<dpdk::Cpu>  cpus;
	std::vector<dpdk::Port> ports;
	dpdk::Mempool           mp;
    Shell                   shell;

	System(int argc, char** argv) : shell(this)
	{
        /*
         * Boot DPDK System.
         */
        kernel_log(SYSTEM, "[+] Booting ...\n");
        print_message();

        /*
         * DPDK init
         */
		int ret = rte_eal_init(argc, argv);
		if (ret < 0) {
			throw slankdev::exception("rte_eal_init");
		}

        kernel_log(SYSTEM, "configure \n");
		uint16_t nb_ports = rte_eth_dev_count();
		uint8_t  nb_cpus  = rte_lcore_count();

        /*
         * Create MemoryPool
         */
		size_t mbuf_cache_size = 0;
		size_t mbuf_siz = RTE_MBUF_DEFAULT_BUF_SIZE;
		size_t num_mbufs = 8192;
		mp.create(
				"Pool0",
				num_mbufs * nb_ports,
				mbuf_cache_size, mbuf_siz,
				rte_socket_id()
		);

        for (size_t i=0; i<nb_cpus; i++)
            cpus.push_back(dpdk::Cpu(i));
        for (size_t i=0; i<nb_ports; i++)
            ports.push_back(dpdk::Port(i, port_bulk_size, &mp, rx_ring_size, tx_ring_size));

        kernel_log(SYSTEM, "[+] DPDK boot Done! \n");
	}
    void halt()
    {
        kernel_log(SYSTEM, "[+] System Halt ...\n");
        rte_exit(0, "Bye...\n");
    }
	void launch()
	{
		/*
		 * The lcore0 is com cpu core.
		 * So it must not launch that.
		 */
        kernel_log(SYSTEM, "launch thread to each-cores \n");
		for (size_t i=1; i<cpus.size(); i++) {
            kernel_log(SYSTEM, "%s lanching ... \n", cpus[i].name.c_str());
		}
        sleep(1);

		for (size_t i=1; i<cpus.size(); i++) {
            cpus[i].launch();
		}
		rte_eal_mp_wait_lcore();
	}
};

size_t System::rx_ring_size   = 128;
size_t System::tx_ring_size   = 512;
size_t System::port_bulk_size = 32;
