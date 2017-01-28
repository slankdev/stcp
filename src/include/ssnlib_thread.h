
#pragma once


namespace ssnlib {

class ssn_thread {
public:
    virtual void operator()() { printf("not set thread \n"); }
    virtual void kill() { printf("not implemented exit()\n"); }
};

} /* namespace ssnlib */
