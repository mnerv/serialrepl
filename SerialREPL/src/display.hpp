#ifndef DISPLAY_HPP_
#define DISPLAY_HPP_

#include <ncurses.h>

namespace sr {

class display {
  public:
    display();
    ~display();

    auto get_width() -> int32_t;
    auto get_height() -> int32_t;

  private:
    int32_t m_width;
    int32_t m_height;
    WINDOW* m_window;
};

}

#endif

