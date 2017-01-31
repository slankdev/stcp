


#include <ssnlib_sys.h>

#include <ssnlib_cmd.h>
#include <ssnlib_log.h>
#include <ssnlib_misc.h>

#include <slankdev/exception.h>
#include <slankdev/util.h>


namespace ssnlib {


size_t System::nb_rx_rings    = 1;
size_t System::nb_tx_rings    = 1;
size_t System::rx_ring_size   = 128;
size_t System::tx_ring_size   = 512;
size_t System::port_bulk_size = 32;


System::System(int argc, char** argv)
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
        cpus.push_back(ssnlib::Cpu(i));
    for (size_t i=0; i<nb_ports; i++) {
        printf("BEGORE\n");
        ports.push_back(
                new ssnlib::Port(i, port_bulk_size, &mp,
                    nb_rx_rings,  nb_tx_rings,
                    rx_ring_size, tx_ring_size)
        );
        printf("AFFTER\n");
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

void System::wait_all()
{
    sleep(1);
    rte_eal_mp_wait_lcore();
}


} /* namespace ssnlib */
