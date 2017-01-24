


#pragma once
#include <string>
#include "mempool.h"
#include "log.h"


namespace dpdk {



using function = int(*)(void*);
using func_arg = void*;

struct Thrd {
	function func;
	func_arg arg;
};

class Cpu {
public:
	const uint8_t lcore_id;
    const std::string name;
    Thrd thrd;

	Cpu(uint8_t id) :
        lcore_id(id),
        name("lcore" + std::to_string(id)),
        thrd({nullptr, nullptr})
    {
        kernel_log(SYSTEM, "boot  %s ... done\n", name.c_str());
    }
    ~Cpu() { rte_eal_wait_lcore(lcore_id); }
	void launch()
	{
		rte_eal_remote_launch(thrd.func, thrd.arg, lcore_id);
	}
};


} /* namespace dpdk */


