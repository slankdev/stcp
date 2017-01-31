

#pragma once

#include <ssnlib_dpdk.h>
#include <ssnlib_mempool.h>
#include <ssnlib_ring.h>
#include <ssnlib_log.h>
#include <ssnlib_port_impl.h>



namespace ssnlib {


class Port {
private:
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

    std::vector<Rxq<Ring>> rxq;
    std::vector<Txq<Ring>> txq;

    port_conf         conf;
    port_stats        stats;
    dev_info          info;
    Mempool           mempool;

    Port() :
        id       (id_next),
        name     ("port" + std::to_string(id)),
        addr     (id),
        conf     (id),
        stats    (id),
        info     (id)
    {
        if (id_next > rte_eth_dev_count()) {
            throw slankdev::exception("invalid port id");
        }

        id_next ++;
        size_t mbuf_cache_size = 0;
        size_t mbuf_siz = RTE_MBUF_DEFAULT_BUF_SIZE;
        size_t num_mbufs = 8192;
        mempool.create(
            name.c_str(),
            num_mbufs ,
            mbuf_cache_size, mbuf_siz,
            rte_socket_id()
        );

        kernel_log(SYSTEM, "boot port%u ... \n", id);
        rte_eth_macaddr_get(id, &addr);
        info.get();

        kernel_log(SYSTEM, "%s address=%s \n", name.c_str(), addr.toString().c_str());

        if (id >= rte_eth_dev_count())
            throw slankdev::exception("port is not exist");

        /*
         * Configure the Ethernet device.
         */
        configure(nb_rx_rings, nb_tx_rings, rx_ring_size, tx_ring_size);

        /*
         * Start the Ethernet port.
         */
        start();

        promiscuous_set(true);
        kernel_log(SYSTEM, "%s configure ... done\n", name.c_str());
    }
    ~Port() { throw slankdev::exception("YUAKDFDKFD\n"); }
    Port(const Port& rhs) = delete;
    Port& operator=(const Port&) = delete;
    void linkup  ()
    {
        int ret = rte_eth_dev_set_link_up  (id);
        if (ret < 0) {
            throw slankdev::exception("rte_eth_dev_link_up: failed");
        }
    }
    void linkdown() { rte_eth_dev_set_link_down(id); }
    void start()
    {
        int ret = rte_eth_dev_start(id);
        if (ret < 0) {
            throw slankdev::exception("rte_eth_dev_start: failed");
        }
    }
    void stop () { rte_eth_dev_stop (id); }
    void promiscuous_set(bool on)
    {
        if (on) rte_eth_promiscuous_enable(id);
        else    rte_eth_promiscuous_disable(id);
    }
    void configure(size_t nb_rx_rings, size_t nb_tx_rings,
            size_t rx_ring_size, size_t tx_ring_size)
    {
        conf.raw.rxmode.mq_mode = ETH_MQ_RX_RSS;
        conf.raw.rx_adv_conf.rss_conf.rss_key = nullptr;
        conf.raw.rx_adv_conf.rss_conf.rss_hf  = ETH_RSS_IP;

        int retval = rte_eth_dev_configure(id, nb_rx_rings, nb_tx_rings, &conf.raw);
        if (retval != 0)
            throw slankdev::exception("rte_eth_dev_configure failed");


        /*
         * Allocate and set up RX $nb_rx_rings queue(s) per Ethernet port.
         */
        int socket_id = rte_socket_id();
        for (uint16_t qid = 0; qid < nb_rx_rings; qid++) {
            retval = rte_eth_rx_queue_setup(id, qid, rx_ring_size,
                    socket_id, NULL, mempool.get_raw());
            if (retval < 0)
                throw slankdev::exception("rte_eth_rx_queue_setup failed");

            std::string ringname = "PORT" + std::to_string(id);
            ringname += "RX" + std::to_string(qid);
            rxq.push_back(Rxq<Ring>(ringname.c_str(), rx_ring_size, socket_id, id, qid));
        }

        /*
         * Allocate and set up $nb_tx_rings TX queue per Ethernet port.
         */
        for (uint16_t qid = 0; qid < nb_tx_rings; qid++) {
            retval = rte_eth_tx_queue_setup(id, qid, tx_ring_size,
                    socket_id, NULL);
            if (retval < 0)
                throw slankdev::exception("rte_eth_tx_queue_setup failed");

            std::string ringname = "PORT" + std::to_string(id);
            ringname += "TX" + std::to_string(qid);
            txq.push_back(Txq<Ring>(ringname.c_str(), tx_ring_size, socket_id, id, qid));
        }

        kernel_log(SYSTEM, "%s configure \n", name.c_str());
        kernel_log(SYSTEM, "  nb_rx_rings=%zd size=%zd\n", nb_rx_rings, rx_ring_size);
        kernel_log(SYSTEM, "  nb_tx_rings=%zd size=%zd\n", nb_tx_rings, tx_ring_size);
    }
};




} /* namespace ssnlib */


