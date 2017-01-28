

#pragma once

#include <ssnlib_mempool.h>
#include <ssnlib_ring.h>

#include <dpdk_struct_utils.h>

#include <susanoo_log.h>





namespace dpdk {




class Port {

    /*
     * This class has dynamically infomations.
     */
    class port_conf {
    public:
        const size_t id;
        rte_eth_conf raw;
        port_conf(size_t i) : id(i)
        {
            memset(&raw, 0x00, sizeof(raw));
            raw.rxmode.max_rx_pkt_len = ETHER_MAX_LEN;
        }
    };

    /*
     * This class has dynamically informations.
     */
    class port_stats {
    public:
        const size_t id;
        struct rte_eth_stats raw;
        port_stats(size_t i) : id(i) {}
        void reset()  { rte_eth_stats_reset(id);       }
        void update() { rte_eth_stats_get  (id, &raw); }
    };

    /*
     * This class has statically infomations.
     */
    class dev_info {
    public:
        const size_t id;
        struct rte_eth_dev_info raw;
        dev_info(size_t i) : id(i) {}
        void get()
        {
            rte_eth_dev_info_get(id, &raw);
        }
    };
    class ether_addr : public ::ether_addr {
    public:
        const size_t id;
        ether_addr(size_t i) : id(i) {}
        void print(FILE* fd) const { fprintf(fd, "%s", toString().c_str()); }
        std::string toString() const
        {
            char buf[32];
            snprintf(buf, sizeof(buf),
                    "%02" PRIx8 ":%02" PRIx8 ":%02" PRIx8
                       ":%02" PRIx8 ":%02" PRIx8 ":%02" PRIx8,
                    addr_bytes[0], addr_bytes[1],
                    addr_bytes[2], addr_bytes[3],
                    addr_bytes[4], addr_bytes[5]);
            return buf;
        }
        void update() { rte_eth_macaddr_get(id, this); }
        void set(::ether_addr* addr)
        {
            int ret = rte_eth_dev_default_mac_addr_set(id, addr);
            if (ret < 0) {
                if (ret == -ENOTSUP) {
                    throw slankdev::exception(
                            "rte_eth_dev_default_mac_addr_set: hardware doesn't suppoer");
                } else if (ret == -ENODEV) {
                    throw slankdev::exception(
                            "rte_eth_dev_default_mac_addr_set: port invalid");
                } else if (ret == -EINVAL) {
                    throw slankdev::exception(
                            "rte_eth_dev_default_mac_addr_set: MAC address is invalid");
                } else {
                    throw slankdev::exception(
                            "rte_eth_dev_default_mac_addr_set: unknown error");
                }
            }
            update();
        }
        void add(::ether_addr* addr)
        {
            int ret = rte_eth_dev_mac_addr_add(id, addr, 0);
            if (ret < 0) {
                if (ret == -ENOTSUP) {
                    throw slankdev::exception(
                    "rte_eth_dev_mac_addr_add: hardware doesn't support this feature.");
                } else if (ret == -ENODEV) {
                    throw slankdev::exception(
                        "rte_eth_dev_mac_addr_add: port is invalid.");
                } else if (ret == -ENOSPC) {
                    throw slankdev::exception(
                        "rte_eth_dev_mac_addr_add: no more MAC addresses can be added.");
                } else if (ret == -EINVAL) {
                    throw slankdev::exception(
                        "rte_eth_dev_mac_addr_add: MAC address is invalid.");
                } else {
                    throw slankdev::exception("rte_eth_dev_mac_addr_add: unknown");
                }
            }
            update();
        }
        void del(::ether_addr* addr)
        {
            int ret = rte_eth_dev_mac_addr_remove(id, addr);
            if (ret < 0) {
                if (ret == -ENOTSUP) {
                    throw slankdev::exception(
                            "rte_eth_dev_mac_addr_remove: hardware doesn't support.");
                } else if (ret == -ENODEV) {
                    throw slankdev::exception(
                            "rte_eth_dev_mac_addr_remove: if port invalid.");
                } else if (ret == -EADDRINUSE) {
                    std::string errstr = "rte_eth_dev_mac_addr_remove: ";
                    errstr += "attempting to remove the default MAC address";
                    throw slankdev::exception(errstr.c_str());
                }
            }
            update();
        }
    };

public:
    const std::string name;
    const size_t      bulk_size;
    const uint8_t     id;
    ether_addr        addr;

    std::vector<Rxq> rxq;
    std::vector<Txq> txq;

    port_conf         conf;
    port_stats        stats;
    dev_info          info;

    Mempool*          mempool;

    Port(uint8_t pid, size_t bs, dpdk::Mempool* mp,
            size_t nb_rx_rings , size_t nb_tx_rings,
            size_t rx_ring_size, size_t tx_ring_size) :
        name     ("port" + std::to_string(pid)),
        bulk_size(bs),
        id       (pid),
        addr     (pid),
        conf     (pid),
        stats    (pid),
        info     (pid),
        mempool  (mp)
    {
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
                    socket_id, NULL, mempool->get_raw());
            if (retval < 0)
                throw slankdev::exception("rte_eth_rx_queue_setup failed");

            std::string ringname = "PORT" + std::to_string(id);
            ringname += "RX" + std::to_string(qid);
            rxq.push_back(Rxq(ringname.c_str(), rx_ring_size, socket_id, id, qid));
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
            txq.push_back(Txq(ringname.c_str(), tx_ring_size, socket_id, id, qid));
        }

        kernel_log(SYSTEM, "%s configure \n", name.c_str());
        kernel_log(SYSTEM, "  nb_rx_rings=%zd size=%zd\n", nb_rx_rings, rx_ring_size);
        kernel_log(SYSTEM, "  nb_tx_rings=%zd size=%zd\n", nb_tx_rings, tx_ring_size);
    }
};




} /* namespace dpdk */


