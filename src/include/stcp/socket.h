

#pragma once 



namespace slank {


class socket {
private:
public:

    socket(int domain, int type, int protocol) {}

    void ioctl(int request, int args)
    {
        // slank::stcp& s = slank::stcp::instance();

        if (request != 0)
            return;

        if (args != 0)
            return;

    }
};



} /* namespace slankdev */
