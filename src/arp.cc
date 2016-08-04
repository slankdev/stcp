

#include <stcp/arp.h>
#include <arpa/inet.h>

#include <stcp/config.h>



static void print_arp_packet(struct arphdr* ah)
{
    // slankdev::hexdump("ARP",  ah, sizeof(struct arphdr));

    printf("ether addr %zd \n", sizeof(struct ether_addr));
    exit(0);
    
    uint8_t d[4];

    printf("hwtype: %u\n", htons(ah->hwtype));
    printf("ptype : 0x%04x\n", htons(ah->ptype));
    printf("hwlen : %u\n", (ah->hwlen    ));
    printf("plen  : %u\n", (ah->plen     ));
    printf("op    : %u\n", htons(ah->operation));

    printf("hwsrc : ");
    for (int i=0; i<6; i++) {
        printf("%02x:", ah->hwsrc.addr_bytes[i]);
    }
    printf("\n");

    // uint32_t_to_char(ah->psrc, d);
    printf("psrc  : %08x", ah->psrc);
    // for (uint8_t o : d) {
    //     printf("%d:", o);
    // }
    printf("\n");

    printf("hwdst : ");
    for (int i=0; i<6; i++) {
        printf("%02x:", ah->hwdst.addr_bytes[i]);
    }
    printf("\n");

    // uint32_t_to_char(ah->pdst, d);
    printf("pdst  : %08x", ah->pdst);
    // for (uint8_t o : d) {
    //     printf("%d:", o);
    // }
    printf("\n");

    return ;
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
