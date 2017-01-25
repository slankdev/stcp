
#pragma once

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

#include <vector>
#include <string>

// #include <slankdev/util.h>
#include <slankdev/exception.h>
#include <susanoo_log.h>

#include "mempool.h"
#include "cpu.h"
#include "port.h"


/* for susanoo_shell */
#include <stdio.h>
#include <stdlib.h>
#include <susanoo_shell.h>
#include <dpdk/system.h>


#define MESGTYPE 5


namespace dpdk {

void print_message();


class System {
    class ssnt_sush : public ssn_thread {
        dpdk::System* sys;
        sush sush0;
    public:
        ssnt_sush(dpdk::System* s) : sys(s) {}
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

	std::vector<Cpu>  cpus;
	std::vector<Port> ports;
	dpdk::Mempool    mp;
    ssnt_sush        shell;

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
            cpus.push_back(Cpu(i));
        for (size_t i=0; i<nb_ports; i++)
            ports.push_back(Port(i, port_bulk_size, &mp, rx_ring_size, tx_ring_size));

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

size_t dpdk::System::rx_ring_size   = 128;
size_t dpdk::System::tx_ring_size   = 512;
size_t dpdk::System::port_bulk_size = 32;




void print_message()
{
    switch (MESGTYPE) {
        case 1:
            kernel_log(SYSTEM, "\n");
            kernel_log(SYSTEM, " oooooooooo.   8@slankdev.   oYoooooooo.   oooo    oooo       .oooooo..o                          \n");
            kernel_log(SYSTEM, " `888'   `Y8b  `888   `Y88. `8U8'   `Y8b  `888   .8P'       d8P'    `Y8         @slankdev         \n");
            kernel_log(SYSTEM, "   888     888  888   .d88'   KB8     888  888  d8'         Y88bo.      oooo    ooo  .oooo.o      \n");
            kernel_log(SYSTEM, "   888     888  888ooo88P'    AO8     888  88888[            `\"Y8888o.   `88.  .8'  d88(  \"8    \n");
            kernel_log(SYSTEM, "   888     888  888           RN8     888  888`88b.              `\"Y88b   `88..8'   `\"Y88b.     \n");
            kernel_log(SYSTEM, "   888    d88'  888           IK8     d88'  888  `88b.       oo     .d8P    `888'    o.  )88b     \n");
            kernel_log(SYSTEM, " o@slankdev'   o888o         o888bood8P'   o888o  o888o      8""88888P'      .8'     8""888P'     \n");
            kernel_log(SYSTEM, "                                                                        .o..P'                    \n");
            kernel_log(SYSTEM, "        @slankdev:please follow me on GitHub                            `Y8P'                     \n");
            kernel_log(SYSTEM, "\n");
            break;
        case 2:
            kernel_log(SYSTEM, "8888888b.  8888888b.  8888888b.  888    d8P       \n");
            kernel_log(SYSTEM, "888  \"Y88b 888   Y88b 888  \"Y88b 888   d8P      \n");
            kernel_log(SYSTEM, "888    888 888    888 888    888 888  d8P         \n");
            kernel_log(SYSTEM, "888    888 888   d88P 888    888 888d88K          \n");
            kernel_log(SYSTEM, "888    888 8888888P\"  888    888 8888888b        \n");
            kernel_log(SYSTEM, "888    888 888        888    888 888  Y88b        \n");
            kernel_log(SYSTEM, "888  .d88P 888        888  .d88P 888   Y88b       \n");
            kernel_log(SYSTEM, "8888888P\"  888        8888888P\"  888    Y88b    \n");
            kernel_log(SYSTEM, "\n");
            break;
        case 5:
            kernel_log(SYSTEM, "+--------------------------------------------------------------------+\n");
            kernel_log(SYSTEM, "| oooooooooo.   8@slankdev.   oYoooooooo.   oooo    oooo             |  \n");
            kernel_log(SYSTEM, "| `888'   `Y8b  `888   `Y88. `8U8'   `Y8b  `888   .8P'               |  \n");
            kernel_log(SYSTEM, "|   888     888  888   .d88'   KB8     888  888  d8'                 |  \n");
            kernel_log(SYSTEM, "|   888     888  888ooo88P'    AO8     888  88888[                   |  \n");
            kernel_log(SYSTEM, "|   888     888  888           RN8     888  888`88b.                 |  \n");
            kernel_log(SYSTEM, "|   888    d88'  888           IK8     d88'  888  `88b.              |  \n");
            kernel_log(SYSTEM, "| o@slankdev'   o888o         o888bood8P'   o888o  o888o             |  \n");
            kernel_log(SYSTEM, "|                                                                    |  \n");
            kernel_log(SYSTEM, "|                                    .oooooo..o                      |    \n");
            kernel_log(SYSTEM, "|                                  d8P'    `Y8         @slankdev     |    \n");
            kernel_log(SYSTEM, "|                                  Y88bo.      oooo    ooo  .oooo.o  |    \n");
            kernel_log(SYSTEM, "|                                   `\"Y8888o.   `88.  .8'  d88(  \"8  |  \n");
            kernel_log(SYSTEM, "| @slankdev:                            `\"Y88b   `88..8'   `\"Y88b.   |  \n");
            kernel_log(SYSTEM, "| please follow me                  oo     .d8P    `888'    o.  )88b |    \n");
            kernel_log(SYSTEM, "| on GitHub Twitter,                8""88888P'      .8'     8""888P'     |\n");
            kernel_log(SYSTEM, "|                                              .o..P'                |    \n");
            kernel_log(SYSTEM, "|                                              `Y8P'                 |    \n");
            kernel_log(SYSTEM, "+--------------------------------------------------------------------+\n");
            break;
        default: throw slankdev::exception("not found");
    }
}



} /* namespace dpdk */


