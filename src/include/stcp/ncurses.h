
#pragma once
#include <ncurses.h>

namespace slank {



class ncurses_native {
private:
    size_t scrn_width;
    size_t scrn_height;
public:
    ncurses_native()
    {
        initscr();
        getmaxyx(stdscr, scrn_height, scrn_width);
        erase();
    }
    ~ncurses_native()
    {
        endwin();
    }
    size_t getw() const { return scrn_width ; }
    size_t geth() const { return scrn_height; }

    template<class... Args>
    static void mvprintw(size_t y, size_t x, const char* fmt, Args... args)
    {
        ::mvprintw(y, x, fmt, args...);
    }
    static void refresh() { ::refresh(); }
    static char getchar() { return ::getch(); }

};


class ncurses : public ncurses_native {
private:
    size_t cur;
public:
    static const size_t POS_PORT  = 0;
    static const size_t POS_ETH   = 10;
    static const size_t POS_ARP   = 12;
    static const size_t POS_IP    = 20;
    static const size_t POS_ICMP  = 28;
    static const size_t POS_UDP   = 30;
    static const size_t POS_TCP   = 32;
    static const size_t POS_DEBUG = 52;

    ncurses() : cur(0) {}
    template<class... Args>
    void mvprintw(size_t y, size_t x, const char* fmt, Args... args)
    {
        ::mvprintw(y, x, fmt, args...);
        refresh();
    }
    template<class... Args>
    void debugprintw(const char* fmt, Args... args)
    {
        cur++;
        ::mvprintw(cur, 0, fmt, args...);
        refresh();
    }
};


} /* namespace slank */
