


#include <stcp/stcp.h>


namespace slank {


core& core::instance()
{
    static core s;
    return s;
}

void core::init(int argc, char** argv)
{
    dpdk.init(argc, argv);
}

void core::ifs_proc()
{
    for (ifnet& dev : dpdk.devices) {
        uint16_t num_reqest_to_send = dev.tx_size();
        uint16_t num_tx = dev.io_tx(num_reqest_to_send);
        if (num_tx != num_reqest_to_send)
            throw exception("some packet droped");

        uint16_t num_rx = dev.io_rx();
        if (unlikely(num_rx == 0)) continue;

        while (dev.rx_size() > 0) {
            mbuf* msg = dev.rx_pop();
            ether.rx_push(msg);
        }
    }
}

void core::run(bool endless)
{
    do {
        ifs_proc();
        ether.proc();
    } while (endless);
}

void core::stat_all()
{
    stat& s = stat::instance();
    s.clean();

    for (ifnet& dev : dpdk.devices) {
        dev.stat();
    }

    s.write("%s", "Ether module");
    s.write("\tRX Packets %zd", ether.rx_cnt);
    s.write("\tTX Packets %zd", ether.tx_cnt);


    s.write("ARP module");
    s.write("\tRX Packets %zd", arp.rx_cnt);
    s.write("\tTX Packets %zd", arp.tx_cnt);
    s.write("");
    s.write("\tWaiting packs  : %zd", arp.wait.size());
    s.write("\tUse dynamic arp: %s", arp.use_dynamic_arp ? "YES" : "NO");
    s.write("");
    s.write("\tARP-chace");
    s.write("\t%-16s %-20s %s", "Address", "HWaddress", "Iface");
    for (stcp_arpreq& a : arp.table) {
        s.write("\t%-16s %-20s %d",
                /* TODO #15 this function will be included in sockaddr-class */
                p_sockaddr_to_str(&a.arp_pa),  
                hw_sockaddr_to_str(&a.arp_ha), a.arp_ifindex);
    }

    s.write("IP module");
    s.write("\tRX Packets %zd", ip.rx_cnt);
    s.write("\tTX Packets %zd", ip.tx_cnt);
    s.write("");
    s.write("\tRouting-Table");
    s.write("\t%-16s%-16s%-16s%-6s%-3s", "Destination", "Gateway", "Genmask", "Flags", "if");
    for (stcp_rtentry& rt : ip.rttable) {
        std::string str_dest;
        if (rt.rt_flags & STCP_RTF_GATEWAY) {
            str_dest = "defalt";
        } else if (rt.rt_flags & STCP_RTF_LOCAL) {
            str_dest = "link-local";
        } else {
            str_dest = p_sockaddr_to_str(&rt.rt_route);
        }

        std::string flag_str = "";
        if (rt.rt_flags & STCP_RTF_GATEWAY  )   flag_str += "G";
        if (rt.rt_flags & STCP_RTF_MASK     )   flag_str += "M";
        if (rt.rt_flags & STCP_RTF_LOCAL    )   flag_str += "L";
        if (rt.rt_flags & STCP_RTF_BROADCAST)   flag_str += "B";

        std::string gateway_str;
        if (rt.rt_flags & STCP_RTF_LOCAL) {
            gateway_str = "*";
        } else {
            gateway_str = p_sockaddr_to_str(&rt.rt_gateway);
        }
        s.write("\t%-16s%-16s%-16s%-6s%-3u",
                str_dest.c_str(),
                gateway_str.c_str(),
                p_sockaddr_to_str(&rt.rt_genmask),
                flag_str.c_str(),
                rt.rt_port);
    }


    s.write("");
    s.write("ICMP module");
    s.write("\tRX Packets %zd", icmp.rx_cnt);
    s.write("\tTX Packets %zd", icmp.tx_cnt);

    s.flush();
}


} /* namespace */
