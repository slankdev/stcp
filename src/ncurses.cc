

#include <stcp/ncurses.h>
#include <unistd.h>
#include <stdlib.h>


namespace stcp {


void ncurses::printframe()
{
    size_t w = scrn_width;
    size_t h = scrn_height;
    for (size_t x=0; x<scrn_width; x++) {
        mvaddch(0  , x, '-');
        mvaddch(h-1, x, '-');
    }

    size_t v = w/2;
    for (size_t y=0; y<scrn_height; y++) {
        mvaddch(y, 0  , '|');
        mvaddch(y, v  , '|');
        mvaddch(y, w-1, '|');
    }

    size_t top = h/2;
    size_t end = h;
    size_t mid = top + (end-top)/2;
    size_t hm  = top + (mid-top)/2;
    size_t lm  = mid + (end-mid)/2;
    for (size_t x=0; x<w/2; x++) {
        mvaddch(top, x, '-');
        mvaddch(mid, x, '-');
        mvaddch(hm , x, '-');
        mvaddch(lm , x, '-');
    }

    mvaddch(0    , 0    , '+');
    mvaddch(0    , w-1  , '+');
    mvaddch(h-1  , 0    , '+');
    mvaddch(h-1  , w-1  , '+');
    mvaddch(0    , w/2  , '+');
    mvaddch(h-1  , w/2  , '+');

    mvaddch(top , 0, '+');
    mvaddch(hm  , 0, '+');
    mvaddch(mid , 0, '+');
    mvaddch(lm  , 0, '+');

    mvaddch(top ,w/2,  '+');
    mvaddch(hm  ,w/2,  '+');
    mvaddch(mid ,w/2,  '+');
    mvaddch(lm  ,w/2,  '+');

    SPACE0 = {0   , 0   };
    SPACE1 = {0   , top };
    SPACE2 = {0   , hm  };
    SPACE3 = {0   , mid };
    SPACE4 = {0   , lm  };
    SPACE5 = {v   , 0   };

    POS_PORT  = {SPACE0.x+1, SPACE0.y+1};
    POS_ARP   = {SPACE1.x+1, SPACE1.y+1};
    POS_IP    = {SPACE2.x+1, SPACE2.y+1};
    POS_ICMP  = {SPACE3.x+1, SPACE3.y+1};
    POS_UDP   = {SPACE4.x+1, SPACE4.y+1};
    POS_TCP   = {SPACE5.x+1, SPACE5.y+1};

    refresh();
}

void ncurses::init()
{
    initscr();
    getmaxyx(stdscr, scrn_height, scrn_width);
    curs_set(0);
    erase();

    size_t w = scrn_width;
    size_t h = scrn_height;
    for (size_t x=0; x<scrn_width; x++) {
        mvaddch(0  , x, '-');
        mvaddch(h-1, x, '-');
    }

    size_t v = w/2;
    for (size_t y=0; y<scrn_height; y++) {
        mvaddch(y, 0  , '|');
        mvaddch(y, v  , '|');
        mvaddch(y, w-1, '|');
    }

    size_t top = h/2;
    size_t end = h;
    size_t mid = top + (end-top)/2;
    size_t hm  = top + (mid-top)/2;
    size_t lm  = mid + (end-mid)/2;
    for (size_t x=0; x<w/2; x++) {
        mvaddch(top, x, '-');
        mvaddch(mid, x, '-');
        mvaddch(hm , x, '-');
        mvaddch(lm , x, '-');
    }

    mvaddch(0    , 0    , '+');
    mvaddch(0    , w-1  , '+');
    mvaddch(h-1  , 0    , '+');
    mvaddch(h-1  , w-1  , '+');
    mvaddch(0    , w/2  , '+');
    mvaddch(h-1  , w/2  , '+');

    mvaddch(top , 0, '+');
    mvaddch(hm  , 0, '+');
    mvaddch(mid , 0, '+');
    mvaddch(lm  , 0, '+');

    mvaddch(top ,w/2,  '+');
    mvaddch(hm  ,w/2,  '+');
    mvaddch(mid ,w/2,  '+');
    mvaddch(lm  ,w/2,  '+');

    SPACE0 = {0   , 0   };
    SPACE1 = {0   , top };
    SPACE2 = {0   , hm  };
    SPACE3 = {0   , mid };
    SPACE4 = {0   , lm  };
    SPACE5 = {v   , 0   };

    POS_PORT  = {SPACE0.x+1, SPACE0.y+1};
    POS_ARP   = {SPACE1.x+1, SPACE1.y+1};
    POS_IP    = {SPACE2.x+1, SPACE2.y+1};
    POS_ICMP  = {SPACE3.x+1, SPACE3.y+1};
    POS_UDP   = {SPACE4.x+1, SPACE4.y+1};
    POS_TCP   = {SPACE5.x+1, SPACE5.y+1};

    refresh();
}

} /* namespace stcp */
