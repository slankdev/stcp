


#include <ssnlib_sys.h>

#include <ssnlib_cmd.h>
#include <ssnlib_log.h>
#include <ssnlib_misc.h>

#include <slankdev/exception.h>
#include <slankdev/util.h>


namespace ssnlib {




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

    for (size_t i=0; i<nb_cpus; i++)
        cpus.push_back(ssnlib::Cpu(i));
    for (size_t i=0; i<nb_ports; i++) {
        ports.push_back( new ssnlib::Port());
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
