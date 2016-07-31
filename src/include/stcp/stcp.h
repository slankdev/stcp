

#pragma once


#include <stcp/dpdk.h>
#include <stcp/config.h>
    

class stcp : public singleton<stcp> {
    friend singleton<stcp>;
    public:
        void init(int argc, char** argv)
        {
            log& log = log::instance();
            log.open("stcp.log");
            log.push("STCP");

            log.write(INFO, "starting...");
            log.write(INFO, "starting all inits");

            dpdk& dpdk = dpdk::instance();
            dpdk.init(argc, argv);

            log.write(INFO, "All inits were finished");

            log.pop();
        }
};


