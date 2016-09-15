

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include <stcp/ifnet.h>
#include <stcp/dpdk.h>
#include <stcp/rte.h>
#include <stcp/stcp.h>


using namespace slank;

void ifnet::init()
{
    struct rte_eth_conf port_conf;
    memset(&port_conf, 0, sizeof port_conf);
    port_conf.rxmode.max_rx_pkt_len = ETHER_MAX_LEN;
    rte::eth_dev_configure(port_id, num_rx_rings, num_tx_rings, &port_conf);

    dpdk& d = dpdk::instance();
    for (uint16_t ring=0; ring<num_rx_rings; ring++) {
        rte::eth_rx_queue_setup(port_id, ring, rx_ring_size,
                rte::eth_dev_socket_id(port_id), NULL, d.get_mempool()); 
    }
    for (uint16_t ring=0; ring<num_tx_rings; ring++) {
        rte::eth_tx_queue_setup(port_id, ring, tx_ring_size,
                rte::eth_dev_socket_id(port_id), NULL); 
    }
    rte::eth_dev_start(port_id);

    if (promiscuous_mode)
        rte::eth_promiscuous_enable(port_id);

    if (rte::eth_dev_socket_id(port_id) > 0 && 
            rte::eth_dev_socket_id(port_id) != (int)rte::socket_id()) {
        char str[128];
        sprintf(str, "WARNING: port %4u is on remote NUMA node to "
                "polling thread. \n\tPerformance will "
                "not be optimal. \n ", port_id);
        throw rte::exception(str);
    }



    struct ether_addr addr;
    rte::eth_macaddr_get(port_id, &addr);
    stcp_sockaddr s;
    for (int i=0; i<6; i++)
        s.sa_data[i] = addr.addr_bytes[i];
    ifaddr ifa(STCP_AF_LINK, &s);
    addrs.push_back(ifa);
}

uint16_t ifnet::io_rx()
{
    struct rte_mbuf* bufs[BURST_SIZE];
    uint16_t num_rx = rte::eth_rx_burst(port_id, 0, bufs, BURST_SIZE);
    if (unlikely(num_rx == 0)) return 0;

    rx.push(array2llist_mbuf(bufs, num_rx));
    rx_packets += num_rx;
    return num_rx;
}

uint16_t ifnet::io_tx(size_t num_request_to_send)
{
    if (num_request_to_send > tx.size()) {
        num_request_to_send = tx.size();
    }

    struct rte_mbuf* bufs[BURST_SIZE];
    uint16_t num_tx_sum = 0;
    size_t i=0;
    for (size_t num_sent=0; num_sent<num_request_to_send; num_sent+=i) {
        for (i=0; i+num_sent<num_request_to_send; i++) {
            bufs[i] = tx.pop();
        }
        uint16_t num_tx = rte::eth_tx_burst(port_id, 0, bufs, i);
        if (num_tx < i) {
            for (uint16_t j=0; j<i-num_tx; j++) {
                rte::pktmbuf_free(bufs[num_tx+j]);
            }
        }
        num_tx_sum += num_tx;
    }

    tx_packets += num_tx_sum;
    return num_tx_sum;
}

void ifnet::stat()
{
    printf("%s: ", name.c_str());
    if (promiscuous_mode) printf("PROMISC ");
    printf("\n");
    printf("\tRX Packets %u Queue %zu\n", rx_packets, rx.size());
    printf("\tTX Packets %u Queue %zu\n", tx_packets, tx.size());
    printf("\tDrop Packets %u \n", drop_packets);

    printf("\n");
    for (ifaddr& ifa : addrs) {
        printf("\t%-10s ", af2str(ifa.family));
        if (ifa.family == STCP_AF_LINK) {
            printf("%02x:%02x:%02x:%02x:%02x:%02x " 
                , ifa.raw.sa_data[0], ifa.raw.sa_data[1]
                , ifa.raw.sa_data[2], ifa.raw.sa_data[3]
                , ifa.raw.sa_data[4], ifa.raw.sa_data[5]);
        } else if (ifa.family == STCP_AF_INET) {
            struct stcp_sockaddr_in* sin = 
                reinterpret_cast<stcp_sockaddr_in*>(&ifa.raw);
            printf("%d.%d.%d.%d " 
                , sin->sin_addr.addr_bytes[0], sin->sin_addr.addr_bytes[1]
                , sin->sin_addr.addr_bytes[2], sin->sin_addr.addr_bytes[3]);
        }
        printf("\n");
    }
    printf("\n");
}


void ifnet::ioctl(uint64_t request, void* arg)
{
    switch (request) {
        case STCP_SIOCSIFADDR:
        {
            stcp_ifreq* ifr = reinterpret_cast<stcp_ifreq*>(arg);
            ioctl_siocsifaddr(ifr);
            break;
        }
        case STCP_SIOCGIFADDR:
        {
            stcp_ifreq* ifr = reinterpret_cast<stcp_ifreq*>(arg);
            ioctl_siocgifaddr(ifr);
            break;
        }
        case STCP_SIOCSIFHWADDR:
        {
            stcp_ifreq* ifr = reinterpret_cast<stcp_ifreq*>(arg);
            ioctl_siocsifhwaddr(ifr);
            break;
        }
        case STCP_SIOCGIFHWADDR:
        {
            stcp_ifreq* ifr = reinterpret_cast<stcp_ifreq*>(arg);
            ioctl_siocgifhwaddr(ifr);
            break;
        }
        case STCP_SIOCSARP:
        {
            const stcp_arpreq* req = reinterpret_cast<const stcp_arpreq*>(arg);
            ioctl_siocsarp(req);
            break;
        }
        default:
        {
            throw slankdev::exception("invalid arguments");
            break;
        }
    }
}


void ifnet::ioctl_siocsifaddr(const stcp_ifreq* ifr)
{
    bool in_addr_setted = false;

    for (size_t i=0; i<addrs.size(); i++) {
        if (addrs[i].family == STCP_AF_INET) {
            const struct stcp_sockaddr_in* sin = 
                reinterpret_cast<const stcp_sockaddr_in*>(&ifr->if_addr);
            stcp_sockaddr_in* s = reinterpret_cast<stcp_sockaddr_in*>(&addrs[i].raw);
            s->sin_addr = sin->sin_addr;
            in_addr_setted = true;
        }
    }
    
    if (in_addr_setted == false) {
        struct ifaddr ifa_new(STCP_AF_INET, &ifr->if_addr);
        addrs.push_back(ifa_new);
    }
}

void ifnet::ioctl_siocgifaddr(stcp_ifreq* ifr)
{
    for (ifaddr ifa : addrs) {
        if (ifa.family == STCP_AF_INET) {
            struct stcp_sockaddr_in* sin = 
                reinterpret_cast<stcp_sockaddr_in*>(&ifr->if_addr);
            struct stcp_sockaddr_in* s =
                reinterpret_cast<stcp_sockaddr_in*>(&ifa.raw);
            sin->sin_fam  = STCP_AF_INET;
            sin->sin_addr = s->sin_addr;
            return;
        }
    }
    throw slankdev::exception("not fount inet address");
}


void ifnet::ioctl_siocsifhwaddr(const stcp_ifreq* ifr)
{
    bool in_addr_setted = false;

    for (size_t i=0; i<addrs.size(); i++) {
        if (addrs[i].family == STCP_AF_LINK) {
            addrs[i].raw.sa_data[0] = ifr->if_hwaddr.sa_data[0];
            addrs[i].raw.sa_data[1] = ifr->if_hwaddr.sa_data[1];
            addrs[i].raw.sa_data[2] = ifr->if_hwaddr.sa_data[2];
            addrs[i].raw.sa_data[3] = ifr->if_hwaddr.sa_data[3];
            addrs[i].raw.sa_data[4] = ifr->if_hwaddr.sa_data[4];
            addrs[i].raw.sa_data[5] = ifr->if_hwaddr.sa_data[5];
            in_addr_setted = true;
        }
    }
    
    if (in_addr_setted == false) {
        struct ifaddr ifa_new(STCP_AF_LINK, &ifr->if_hwaddr);
        addrs.push_back(ifa_new);
    }
}


void ifnet::ioctl_siocgifhwaddr(stcp_ifreq* ifr)
{
    for (ifaddr ifa : addrs) {
        if (ifa.family == STCP_AF_LINK) {
            ifr->if_hwaddr.sa_data[0] = ifa.raw.sa_data[0];
            ifr->if_hwaddr.sa_data[1] = ifa.raw.sa_data[1];
            ifr->if_hwaddr.sa_data[2] = ifa.raw.sa_data[2];
            ifr->if_hwaddr.sa_data[3] = ifa.raw.sa_data[3];
            ifr->if_hwaddr.sa_data[4] = ifa.raw.sa_data[4];
            ifr->if_hwaddr.sa_data[5] = ifa.raw.sa_data[5];
            return;
        }
    }
    throw slankdev::exception("not fount inet address");
}



void ifnet::ioctl_siocsarp(const stcp_arpreq* req)
{
    struct ether_addr ha;
    for (int i=0; i<6; i++)
        ha.addr_bytes[i] = req->arp_ha.sa_data[i];
    const stcp_sockaddr_in* pa = reinterpret_cast<const stcp_sockaddr_in*>(&req->arp_pa);

    struct stcp_arphdr ah;
    ah.operation = 2;
    ah.hwsrc = ha;
    ah.psrc  = pa->sin_addr;

    core& c = core::instance();
    c.arp.proc_update_arptable(&ah, req->arp_ifindex);
}


