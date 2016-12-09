
#pragma once
#include <ncurses.h>

namespace slank {



class ncurses_native {
private:
    size_t scrn_width;
    size_t scrn_height;

    size_t cur_x;
    size_t cur_y;
public:
    ncurses_native() {}
    ~ncurses_native()
    {
        endwin();
    }
    void init()
    {
        initscr();
        getmaxyx(stdscr, scrn_height, scrn_width);
        curs_set(0);
        erase();
    }
    size_t getw() const { return scrn_width ; }
    size_t geth() const { return scrn_height; }

    static void refresh() { ::refresh(); }
    static char getchar() { return ::getch(); }
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
};

struct position {
    size_t x;
    size_t y;
};


class ncurses : public ncurses_native {
private:
public:
    const position POS_PORT  = {0 , 0 };
    const position POS_ETH   = {0 , 10};
    const position POS_ARP   = {0 , 12};
    const position POS_IP    = {0 , 20};
    const position POS_ICMP  = {0 , 28};
    const position POS_UDP   = {0 , 30};
    const position POS_TCP   = {70, 0 };
    const position POS_STDO  = {0 , 52};
    const position POS_DEBUG = {0 , 62};

    ncurses() {}
    template<class... Args>
    void mvprintw(size_t y, size_t x, const char* fmt, Args... args)
    {
        ::mvprintw(y, x, fmt, args...);
        refresh();
    }
    template<class... Args>
    void debugprintw(const char* fmt, Args... args)
    {
        static size_t cur = 0;
        cur++;
        if (cur > 10) cur = 0;
        ::mvprintw(POS_DEBUG.y + cur, POS_DEBUG.x, fmt, args...);
        refresh();
    }
};


} /* namespace slank */
