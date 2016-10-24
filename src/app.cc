
#include <stcp/app.h>


namespace slank {


stcp_app::stcp_app()
{
    core::apps.push_back(this);
}

stcp_app::~stcp_app()
{
    for (size_t i=0; i<core::apps.size(); i++) {
        if (core::apps[i] == this) {
            core::apps.erase(core::apps.begin() + i);
        }
    }
}

} /* namespace slank */
