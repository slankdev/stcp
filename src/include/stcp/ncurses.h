
#pragma once
#include <ncurses.h>

namespace slank {



struct position {
    size_t x;
    size_t y;
};



class ncurses {
protected:
    size_t scrn_width;
    size_t scrn_height;

    size_t cur_x;
    size_t cur_y;

public:
    position POS_PORT;
    position POS_ETH ;
    position POS_ARP ;
    position POS_IP  ;
    position POS_ICMP;
    position POS_UDP ;
    position POS_TCP ;

    position SPACE0;
    position SPACE1;
    position SPACE2;
    position SPACE3;
    position SPACE4;
    position SPACE5;

    ncurses() {}
    ~ncurses()
    {
        endwin();
    }
    void init();
    void erase() { ::erase(); }
    void refresh() { ::refresh(); }
    void move(size_t y, size_t x)
    {
        cur_y = y;
        cur_x = x;
        ::move(y, x);
    }
    void indention()
    {
        cur_y++;
        ::move(cur_y, cur_x);
    }
    template<class... Args>
    void mvprintw(size_t y, size_t x, const char* fmt, Args... args)
    {
        ::mvprintw(y, x, fmt, args...);
    }
    template<class... Args>
    void printw(const char* fmt, Args... args)
    {
        ::printw(fmt, args...);
    }
    template<class... Args>
    void printwln(const char* fmt, Args... args)
    {
        ::printw(fmt, args...);
        indention();
    }
    void printframe();
};



} /* namespace slank */
