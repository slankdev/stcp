
#pragma once

#include <dpdk/system.h>


namespace api {

static void ifconfig(dpdk::System* sys)
{
    for (dpdk::Port& port : sys->ports) {
        port.stats.update();

        printf("%s\n", port.name.c_str());
        printf("  HWaddr %s \n", port.addr.toString().c_str());

        auto& stats = port.stats;
        printf("  RX packets:%lu errors:%lu dropped:%lu allocmiss:%lu \n",
                    stats.raw.ipackets, stats.raw.ierrors,
                    stats.raw.imissed, stats.raw.rx_nombuf);
        printf("  TX packets:%lu errors:%lu  \n",
                stats.raw.opackets, stats.raw.oerrors);
        printf("  RX bytes:%lu TX bytes:%lu \n", stats.raw.ibytes, stats.raw.obytes);
        printf("  RX ring:%zd/%zd TX ring:%zd/%zd\n",
                port.rxq[0].count(), port.rxq[0].size(),
                port.txq[0].count(), port.txq[0].size());

    }
}


} /* namespace api */
