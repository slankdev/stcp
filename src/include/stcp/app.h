
#pragma once


#include <stcp/stcp.h>



namespace slank {



class stcp_app {
public:
    stcp_app();
    ~stcp_app();
    virtual void proc() {}
};



} /* namespace slank */
