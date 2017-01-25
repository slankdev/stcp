

#pragma once

#include <vector>
#include <string>



class Command {
public:
    std::string name;
    virtual void operator()(const std::vector<std::string>& args) = 0;
    virtual ~Command() {}
};

