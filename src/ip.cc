



#include <stcp/ip.h>
#include <stcp/socket.h>

namespace slank {




const char* stcp_rtentry::c_str()
{
    // TODO: Discriminate against DefaultGW
    // TODO implement!!
    // XXX string update error.
    static std::string str;

    std::string str_dest;
    if (rt_flags & STCP_RTF_GATEWAY) {
        str_dest = "defalt";
    } else if (rt_flags & STCP_RTF_LOCAL) {
        str_dest = "link-local";
    } else {
        str_dest = p_sockaddr_to_str(&rt_route);
    }


    std::string str_flag;

    size_t strlength = 16+16+16+6+3; // XXX hardcode
    str.resize(strlength+1);
    std::fill(str.begin(), str.end(), 0);
    sprintf(&str[0], "%-16s%-16s%-16s%-6s%-3u", 
            str_dest.c_str(),
            p_sockaddr_to_str(&rt_gateway),
            p_sockaddr_to_str(&rt_genmask),
            str_flag.c_str(),
            rt_port
    );
    printf("%-16s%-16s%-16s%-6s%-3u\n", 
            str_dest.c_str(),
            p_sockaddr_to_str(&rt_gateway),
            p_sockaddr_to_str(&rt_genmask),
            str_flag.c_str(),
            rt_port
    );
    slankdev::hexdump("", &rt_genmask, sizeof(stcp_sockaddr));
    exit(-1);
    str.resize(strlen(&str[0]));

    return str.c_str();
}




void ip_module::stat()
{
    m.stat();
    printf("\n");
    printf("\tRouting-Table\n");
    printf("\t%-16s%-16s%-16s%-6s%-3s\n", "Destination", "Gateway", "Genmask", "Flags", "if");
    for (stcp_rtentry& rt : rttable) {
        printf("\t%s\n", rt.c_str());
    }
}


void ip_module::ioctl(uint64_t request, void* args)
{
    switch (request) {
        case STCP_SIOCADDRT:
        {
            const stcp_rtentry* rt = reinterpret_cast<const stcp_rtentry*>(args); 
            ioctl_siocaddrt(rt);
            break;
        }
        case STCP_SIOCADDGW:
        {
            stcp_rtentry* rt = reinterpret_cast<stcp_rtentry*>(args); 
            ioctl_siocaddgw(rt);
            break;
        }
        default:
        {
            throw slankdev::exception("invalid arguments");
            break;
        }
    }
}





void ip_module::ioctl_siocaddrt(const stcp_rtentry* rt)
{
    rttable.push_back(*rt);
}


/*
 * This function evaluate only these.
 *  - rt.rt_port
 *  - rt.rt_gateway
 */
void ip_module::ioctl_siocaddgw(stcp_rtentry* rt)
{
    rt->rt_route.inet_addr(0, 0, 0, 0);
    rt->rt_genmask.inet_addr(0, 0, 0, 111);
    rt->rt_flags = STCP_RTF_GATEWAY;
    rttable.push_back(*rt);
}


};
