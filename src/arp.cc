

#include <stcp/arp.h>
#include <arpa/inet.h>

#include <stcp/config.h>

#include <pgen2.h>



static void print_arp_packet(struct arphdr* ah)
{
    pgen::arp_header e;
    e.read(ah, 28);
    e.summary(true);

    printf("\n");
    for (int i=0; i<4; i++) {
        printf("%x:", ah->psrc.addr_bytes[i]);
    }
    printf("\n");
    printf("\n");

    return;
}


void arp_module::proc() 
{
    while (m.rx_size() > 0) {
        struct rte_mbuf* msg = m.rx_pop();
        struct arphdr* ah  = rte::pktmbuf_mtod<struct arphdr*>(msg);
        print_arp_packet(ah);
        m.drop(msg);
    }
}
