

#pragma once

#include <unistd.h>

#include <ssnlib_dpdk.h>
#include <ssnlib_mempool.h>
#include <ssnlib_ring.h>
#include <ssnlib_log.h>
#include <ssnlib_port_impl.h>


namespace ssnlib {


class Port_interface {
protected:
    static size_t id_next;

public:
    static size_t nb_rx_rings;
    static size_t nb_tx_rings;
    static size_t rx_ring_size;
    static size_t tx_ring_size;

public:
    const uint8_t     id;
    const std::string name;
    const size_t      bulk_size = 32;
    ether_addr        addr;

    port_conf         conf;
    port_stats        stats;
    dev_info          info;

public:
    Port_interface() :
        id       (id_next++),
        name     ("port" + std::to_string(id)),
        addr     (id),
        conf     (id),
        stats    (id),
        info     (id)
    {
        if (id_next > rte_eth_dev_count()) {
            throw slankdev::exception("invalid port id");
        }


        kernel_log(SYSTEM, "boot port%u ... \n", id);
        rte_eth_macaddr_get(id, &addr);
        info.get();

        kernel_log(SYSTEM, "%s address=%s \n", name.c_str(), addr.toString().c_str());

        if (id >= rte_eth_dev_count())
            throw slankdev::exception("port is not exist");
    }
    ~Port_interface() { throw slankdev::exception("YUAKDFDKFD\n"); }
    virtual void boot()
    {
        configure();
        start();
        promiscuous_set(true);
        kernel_log(SYSTEM, "%s configure ... done\n", name.c_str());
    }
    virtual void linkup  ()
    {
        int ret = rte_eth_dev_set_link_up  (id);
        if (ret < 0) {
            throw slankdev::exception("rte_eth_dev_link_up: failed");
        }
    }
    virtual void start()
    {
        int ret = rte_eth_dev_start(id);
        if (ret < 0) {
            throw slankdev::exception("rte_eth_dev_start: failed");
        }
    }
    virtual void promiscuous_set(bool on)
    {
        if (on) rte_eth_promiscuous_enable(id);
        else    rte_eth_promiscuous_disable(id);
    }
    virtual void configure() = 0;
};
size_t Port_interface::id_next = 0;
size_t Port_interface::nb_rx_rings    = 1;
size_t Port_interface::nb_tx_rings    = 1;
size_t Port_interface::rx_ring_size   = 128;
size_t Port_interface::tx_ring_size   = 512;



template <class RXQ=Rxq, class TXQ=Txq>
class Port : public Port_interface {

public:
    std::vector<RXQ> rxq;
    std::vector<TXQ> txq;

    void linkdown() { rte_eth_dev_set_link_down(id); }
    void stop () { rte_eth_dev_stop (id); }


    void configure() override
    {
        conf.raw.rxmode.mq_mode = ETH_MQ_RX_RSS;
        conf.raw.rx_adv_conf.rss_conf.rss_key = nullptr;
        conf.raw.rx_adv_conf.rss_conf.rss_hf  = ETH_RSS_IP;

        int retval = rte_eth_dev_configure(id, nb_rx_rings, nb_tx_rings, &conf.raw);
        if (retval != 0)
            throw slankdev::exception("rte_eth_dev_configure failed");

        rxq.reserve(nb_tx_rings);
        for (uint16_t qid=0; qid<nb_rx_rings; qid++) {
            rxq.emplace_back(id, qid, rx_ring_size);
        }
        txq.reserve(nb_tx_rings);
        for (uint16_t qid=0; qid<nb_tx_rings; qid++) {
            txq.emplace_back(id, qid, tx_ring_size);
        }

        kernel_log(SYSTEM, "%s configure \n", name.c_str());
        kernel_log(SYSTEM, "  nb_rx_rings=%zd size=%zd\n", nb_rx_rings, rx_ring_size);
        kernel_log(SYSTEM, "  nb_tx_rings=%zd size=%zd\n", nb_tx_rings, tx_ring_size);
    }
};




} /* namespace ssnlib */


