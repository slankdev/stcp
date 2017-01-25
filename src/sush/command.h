

#pragma once

#include <vector>
#include <string>



class Command {
public:
    std::string name;
    virtual void operator()() = 0;
    virtual ~Command() {}
};

