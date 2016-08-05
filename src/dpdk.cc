

#include <stdint.h>
#include <stddef.h>
#include <stcp/dpdk.h>

using namespace slank;


uint32_t dpdk::num_mbufs = 8192;
uint32_t dpdk::mbuf_cache_size = 250;
std::string dpdk::mp_name = "STCP";

void dpdk::init(int argc, char** argv)
{
    log& log = log::instance();
    log.push("DPDK");

    rte::eth_dev_init(argc, argv);
    log.write(INFO, "create memry pool as %s", mp_name.c_str());
    mempool = rte::pktmbuf_pool_create(
            mp_name.c_str(), 
            num_mbufs * rte::eth_dev_count(), 
            mbuf_cache_size, 
            0, 
            RTE_MBUF_DEFAULT_BUF_SIZE, 
            rte::socket_id()
            );

    for (size_t port=0; port<rte::eth_dev_count(); port++) {
        ifnet dev(port);
        dev.init();
        devices.push_back(dev);
    }
    log.pop();
}
