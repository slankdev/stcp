



#include <stcp/ip.h>
#include <stcp/socket.h>

namespace slank {




const char* stcp_rtentry::c_str()
{
    // TODO: Discriminate against DefaultGW
    // TODO implement!!
    static std::string str;
    std::string str_dest;
        = ((rt_flags&STCP_RTF_GATEWAY)!=0) ? "default" : p_sockaddr_to_str(&rt_route);
    std::string str_flag;

    size_t strlength = 16+16+16+6+3; // XXX hardcode
    str.resize(strlength+1);
    std::fill(str.begin(), str.end(), 0);
    sprintf(&str[0], "%-16s%-16s%-16s%-6s%-3u", 
            str_dest.c_str(),
            p_sockaddr_to_str(&rt_gateway),
            p_sockaddr_to_str(&rt_mask),
            str_flag.c_str(),
            rt_port
    );
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
        case STCP_SIOCSETGW:
        {
            const stcp_rtentry* rt = reinterpret_cast<const stcp_rtentry*>(args); 
            ioctl_siocsetgw(rt);
            break;
        }
        default:
        {
            throw slankdev::exception("invalid arguments");
            break;
        }
    }
}



void ip_module::ioctl_siocsetgw(const stcp_rtentry* rt)
{
    // TODO velify routeing information.
    rttable.push_back(*rt);
}




};
