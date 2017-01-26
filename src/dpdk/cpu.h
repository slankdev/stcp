


#pragma once
#include <string>

#include "mempool.h"
#include <susanoo_log.h>


class ssn_thread {
public:
    virtual void operator()()
    {
        printf("not set thread \n");
    }
};


namespace dpdk {


int Exe(void* arg);


class Cpu {
public:
	const uint8_t lcore_id;
    const std::string name;
    ssn_thread* thrd;

	Cpu(uint8_t id) :
        lcore_id(id),
        name("lcore" + std::to_string(id)),
        thrd(nullptr)
    {
        kernel_log(SYSTEM, "boot  %s ... done\n", name.c_str());
    }
    ~Cpu() { rte_eal_wait_lcore(lcore_id); }
	void launch()
	{
        if (thrd) {
            if (lcore_id == 0) {
                (*thrd)();
            } else {
                rte_eal_remote_launch(Exe, this, lcore_id);
            }
        }

	}
};

int Exe(void* arg)
{
    dpdk::Cpu* cpu = reinterpret_cast<dpdk::Cpu*>(arg);
    (*cpu->thrd)();
    return 0;
}


} /* namespace dpdk */


