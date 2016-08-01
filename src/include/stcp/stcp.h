

#pragma once


#include <stcp/dpdk.h>
#include <stcp/config.h>
    

class stcp {
    private:
        stcp() {}
        ~stcp() {}
        stcp(const stcp&) = delete;
        stcp& operator=(const stcp&) = delete;

    public:
        static stcp& instance()
        {
            static stcp s;
            return s;
        }

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


