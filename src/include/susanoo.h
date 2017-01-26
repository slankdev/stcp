

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
#include <slankdev/util.h>

#include <susanoo_log.h>
#include <susanoo_misc.h>

#include <dpdk/mempool.h>
#include <dpdk/cpu.h>
#include <dpdk/port.h>


/* for susanoo_shell */
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>


static inline char* Readline(const char* p)
{
    char* line = readline(p);
    add_history(line);
    return line;
}




class System {
public:
    class Command {
    public:
        std::string name;
        virtual void operator()(const std::vector<std::string>& args) = 0;
        virtual ~Command() {}
    };

    class Cmd_version : public Command {
    public:
        Cmd_version() { name = "version"; }
        void operator()(const std::vector<std::string>& args)
        {
            UNUSED(args);
            printf("Susanoo version 0.0 \n");
        }
    };

    class Cmd_quit : public Command {
        System* sys;
    public:
        Cmd_quit(System* s) : sys(s) { name = "quit"; }
        void operator()(const std::vector<std::string>& args)
        {
            UNUSED(args);
            sys->halt();
        }
    };

    class Shell : public ssn_thread {
        std::vector<Command*> cmds;
        System* sys;
    public:

        Shell(System* s) : sys(s)
        {
            add_cmd(new Cmd_version()   );
            add_cmd(new Cmd_quit   (sys));
        }
        ~Shell() { for (Command* cmd : cmds) delete(cmd); }
        void add_cmd(Command* newcmd)
        {
            cmds.push_back(newcmd);
        }
        void exe_cmd(const char* cmd_str)
        {
            if (strlen(cmd_str) == 0) return;
            std::vector<std::string> args = slankdev::split(cmd_str, ' ');
            for (Command* cmd : cmds) {
                if (cmd->name == args[0]) {
                    (*cmd)(args);
                    return;
                }
            }
            printf("SUSH: command not found: %s\n", args[0].c_str());
        }
        void operator()()
        {
            printf("\n\n");
            const char* prmpt = "SUSANOO$ ";
            while (char* line = Readline(prmpt)) {
                exe_cmd(line);
                free(line);
            }
            return;
        }
    };

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
        for (size_t i=0; i<nb_ports; i++) {
            ports.push_back(
                    dpdk::Port(i, port_bulk_size, &mp,
                        nb_rx_rings,  nb_tx_rings,
                        rx_ring_size, tx_ring_size)
            );
        }

        kernel_log(SYSTEM, "[+] DPDK boot Done! \n");
	}
    ~System() { rte_eal_mp_wait_lcore(); }
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
        cpus[0].launch();
	}
};

size_t System::nb_rx_rings    = 1;
size_t System::nb_tx_rings    = 1;
size_t System::rx_ring_size   = 128;
size_t System::tx_ring_size   = 512;
size_t System::port_bulk_size = 32;
