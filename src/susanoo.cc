


#include <susanoo.h>
#include <susanoo_cmd.h>
#include <susanoo_log.h>
#include <susanoo_misc.h>

#include <slankdev/exception.h>
#include <slankdev/util.h>



size_t System::nb_rx_rings    = 1;
size_t System::nb_tx_rings    = 1;
size_t System::rx_ring_size   = 128;
size_t System::tx_ring_size   = 512;
size_t System::port_bulk_size = 32;


System::System(int argc, char** argv) : shell(this)
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

    for (size_t i=0; i<RTE_CPUFLAG_NUMFLAGS; i++)
        cpuflags[i] = rte_cpu_get_flag_name(rte_cpu_flag_t(i));

    kernel_log(SYSTEM, "[+] DPDK boot Done! \n");
}

void System::halt()
{
    kernel_log(SYSTEM, "[+] System Halt ...\n");
    rte_exit(0, "Bye...\n");
}

void System::launch()
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


