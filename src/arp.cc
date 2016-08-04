

#include <arpa/inet.h>

#include <stcp/arp.h>
#include <stcp/config.h>

#include <pgen2.h>





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


void arp_module::update_table(arpentry newent)
{
    for (arpentry& ent : table) {
        if (ent.port == newent.port) {
            if (ent.ip == newent.ip) {
                if (is_same(ent.mac, newent.mac)) {
                    continue;
                } else {
                    ent.mac = newent.mac;
                    continue;
                }
            } else { /* ip isnt same */

            }
        } else { /* port isnt same */

        }
    }

}

void arp_module::proc() 
{
    // procc recv packet
    while (m.rx_size() > 0) {
        struct rte_mbuf* msg = m.rx_pop();
        struct arphdr* ah  = rte::pktmbuf_mtod<struct arphdr*>(msg);
        uint8_t port = msg->port;

        if (ah->operation == htons(2)) { // TODO hard code
            arpentry newent(ah->psrc, ah->hwsrc, port);
            update_table(newent);
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
    for (arpentry& a : table) {
        printf("\t%-16s %-20s %d\n", ip2cstr(a.ip), mac2cstr(a.mac), a.port);
    }
}
