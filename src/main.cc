


#include <stcp/dpdk.h>
#include <stcp/rte.h>
#include <stcp/stcp.h>




static void l2_repeater_hub(int argc, char** argv)
{

    stcp& s = stcp::instance();  
    s.init(argc, argv);
    s.run();
}



int main(int argc, char** argv)
{
    l2_repeater_hub(argc, argv);
}

