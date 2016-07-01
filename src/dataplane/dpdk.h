
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include <rte_config.h>
#include <rte_version.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_hexdump.h>


class dpdk {
    private:

    public:
        struct rte_mempool* mbuf_pool;



        void eth_dev_init(int argc, char** argv);
        void eth_dev_configure(uint8_t port_id, uint16_t nb_rx_queue,
                uint16_t nb_tx_queue, const struct rte_eth_conf* eth_conf);
        void eth_rx_queue_setup(uint8_t port_id, 
                uint16_t rx_queue_id, 
                uint16_t nb_rx_desc, uint32_t socket_id, 
                const struct rte_eth_rxconf* rx_conf);
        void eth_tx_queue_setup(uint8_t port_id, 
                uint16_t tx_queue_id, 
                uint16_t nb_tx_desc, uint32_t socket_id, 
                const struct rte_eth_txconf* tx_conf);
        void eth_dev_start(uint8_t);
        void eth_promiscuous_enable(uint8_t);
        void pktmbuf_pool_create(const char* name, uint32_t n, 
                uint32_t cache_size, uint16_t priv_size, 
                uint16_t data_room_size, int socket_id);
        size_t eth_dev_count();
        size_t lcore_count();
        
        void eth_macaddr_get(uint8_t port, struct ether_addr* addr);
        uint32_t socket_id();

    public:
        dpdk();
        ~dpdk();

        void port_init();
};



namespace rte {



/* This class provides config of ether_port */
#include <rte_ethdev.h>



class eth_rxmode {
    public:
        enum rte_eth_rx_mq_mode mq_mode;
        uint32_t max_rx_pkt_len;  
        uint16_t split_hdr_size;  
        uint16_t header_split : 1, 
                 hw_ip_checksum   : 1, 
                 hw_vlan_filter   : 1, 
                 hw_vlan_strip    : 1, 
                 hw_vlan_extend   : 1, 
                 jumbo_frame      : 1, 
                 hw_strip_crc     : 1, 
                 enable_scatter   : 1, 
                 enable_lro       : 1; 
    public:
        eth_rxmode() : 
            mq_mode((rte_eth_rx_mq_mode)0),
            max_rx_pkt_len(ETHER_MAX_LEN), 
            split_hdr_size(0) {}
        
};


class eth_txmode {
    public:
        enum rte_eth_tx_mq_mode mq_mode; 
        uint16_t pvid;
        uint8_t hw_vlan_reject_tagged : 1,
                hw_vlan_reject_untagged : 1,
                hw_vlan_insert_pvid : 1;
    public:
        eth_txmode() :
            mq_mode((rte_eth_tx_mq_mode)0),
            pvid(0) {}
};

class eth_conf {
    public:
        uint32_t link_speeds; 
        rte::eth_rxmode rxmode; 
        struct rte_eth_txmode txmode; 
        uint32_t lpbk_mode; 
        struct {
            struct rte_eth_rss_conf rss_conf; 
            struct rte_eth_vmdq_dcb_conf vmdq_dcb_conf;
            struct rte_eth_dcb_rx_conf dcb_rx_conf;
            struct rte_eth_vmdq_rx_conf vmdq_rx_conf;
        } rx_adv_conf; 
        union {
            struct rte_eth_vmdq_dcb_tx_conf vmdq_dcb_tx_conf;
            struct rte_eth_dcb_tx_conf dcb_tx_conf;
            struct rte_eth_vmdq_tx_conf vmdq_tx_conf;
        } tx_adv_conf; 
        uint32_t dcb_capability_en;
        struct rte_fdir_conf fdir_conf; 
        struct rte_intr_conf intr_conf; 

    public:
        eth_conf() : link_speeds(0), lpbk_mode(0), dcb_capability_en(0) 
        {
            memset(&rx_adv_conf, 0, sizeof rx_adv_conf);
            memset(&tx_adv_conf, 0, sizeof rx_adv_conf);
            memset(&fdir_conf, 0, sizeof fdir_conf);
            memset(&intr_conf, 0, sizeof intr_conf);
        }
};




} /* namespace */


