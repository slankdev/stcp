

#pragma once


struct arphdr {

    
};


#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include <stcp/stcp.h>
#include <stcp/rte.h>
#include <stcp/config.h>




class proto_module {
public:
    pkt_queue rx;
    pkt_queue tx;
};
