
#include <stcp/stcp.h>

using namespace slank;

int main(int argc, char** argv)
{
    stcp& s = stcp::instance();  
    s.init(argc, argv);
    s.run();
}

