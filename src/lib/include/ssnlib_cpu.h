


#pragma once
#include <string>

#include <ssnlib_log.h>
#include <ssnlib_mempool.h>
#include <ssnlib_thread.h>



namespace ssnlib {



class Cpu {
    static int Exe(void* arg)
    {
        ssnlib::Cpu* cpu = reinterpret_cast<ssnlib::Cpu*>(arg);
        (*cpu->thread)();
        return 0;
    }

public:
	const uint8_t lcore_id;
    const std::string name;
    ssn_thread* thread;

	Cpu(uint8_t id) :
        lcore_id(id),
        name("lcore" + std::to_string(id)),
        thread(nullptr)
    {
        kernel_log(SYSTEM, "boot  %s ... done\n", name.c_str());
    }
    Cpu(const Cpu& rhs) = delete;
    Cpu& operator=(const Cpu& rhs) = delete;
    ~Cpu() { rte_eal_wait_lcore(lcore_id); }
	void launch()
	{
        if (thread) {
            if (lcore_id == 0) {
                fprintf(stderr, "This is COM core. can not launch thread");
            } else {
                kernel_log(SYSTEM, "%s lanching ... ", name.c_str());
                rte_eal_remote_launch(Cpu::Exe, this, lcore_id);
                printf("done \n");
            }
        }
	}
    rte_lcore_state_t get_state()
    {
        return rte_eal_get_lcore_state(lcore_id);
    }
};



} /* namespace ssnlib */

