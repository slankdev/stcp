


// #include <stcp/dpdk.h>
// #include <stcp/rte.h>
#include <stcp/stcp.h>






int main(int argc, char** argv)
{
    stcp& s = stcp::instance();  
    s.init(argc, argv);
    s.run();
}

