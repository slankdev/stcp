
#pragma once


namespace ssnlib {

class ssn_thread {
public:
    virtual void operator()() { printf("not set thread \n"); }
};

} /* namespace ssnlib */
