

#include <arpa/inet.h>

#include <stcp/arp.h>
#include <stcp/config.h>

#include <pgen2.h>


using namespace slank;


static char* ip2cstr(const struct ip_addr ip)
{
    static char str[16];
    sprintf(str, "%d.%d.%d.%d", 
            ip.addr_bytes[0], ip.addr_bytes[1],
            ip.addr_bytes[2], ip.addr_bytes[3]);
    return str;
}
static char* mac2cstr(const struct ether_addr mac)
{
    static char str[32];
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", 
            mac.addr_bytes[0], mac.addr_bytes[1],
            mac.addr_bytes[2], mac.addr_bytes[3],
            mac.addr_bytes[4], mac.addr_bytes[5]);
    return str;
}


static bool is_same(struct ether_addr& a, struct ether_addr& b)
{
    for (int i=0; i<6; i++) 
        if (a.addr_bytes[i] != b.addr_bytes[i])
            return false;
    return true;
}


void arp_module::update_table(arpentry newent, uint8_t port)
{
    for (arpentry& ent : table[port].entrys) {
        if (ent.ip == newent.ip) {
            if (is_same(ent.mac, newent.mac)) {
                return;
            } else {
                ent.mac = newent.mac;
                return;
            }
        } else { /* ip isnt same */
            continue;
        }
    }
    table[port].entrys.push_back(newent);
}


void arp_module::proc() 
{
    while (m.rx_size() > 0) {
        struct rte_mbuf* msg = m.rx_pop();
        struct arphdr* ah  = rte::pktmbuf_mtod<struct arphdr*>(msg);
        uint8_t port = msg->port;

        if (ah->operation == htons(2)) { // TODO hard code
            arpentry newent(ah->psrc, ah->hwsrc);
            update_table(newent, port);
        } else if (ah->operation == htons(1)) {
            // TODO 次回はここから!!!!!!
            // printf("recv arp request \n");
            // exit(-1);
        }
        rte::pktmbuf_free(msg);
    }
}




void arp_module::stat() 
{
    m.stat();
    printf("\n");
    printf("\tARP-chace\n");
    printf("\t%-16s %-20s %s\n", "Address", "HWaddress", "Iface");
    for (size_t i=0; i<table.size(); i++) {
        for (arpentry& a : table[i].entrys)
            printf("\t%-16s %-20s %zd\n", ip2cstr(a.ip), mac2cstr(a.mac), i);
    }
}


