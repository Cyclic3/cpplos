#if 0

#include <terminal.hpp>

#include <ncurses.h>

namespace cpplos {
  terminal term = detail::make_terminal();

  struct terminal::impl {
    ::WINDOW* window;
    std::string wip;
  };

  std::variant<terminal::click_t, char> terminal::get_next_event(bool echo) {
    MEVENT event;
    int c = getch(); //wgetch(_impl->window);

    if (c == KEY_MOUSE) {
      if (getmouse(&event))
        throw std::logic_error{"Ncurses incorrectly reported a mouse movement"};
      click_t ret;
      ret.x = event.x;
      ret.y = event.y;
      if (event.bstate & BUTTON1_CLICKED)
        ret.button = click_t::Left;
      else if (event.bstate & BUTTON3_CLICKED)
        ret.button = click_t::Right;
      else // Dunno, try again
        return get_next_event();

      ret.count = 1; // TODO: actually work this out

      return ret;
    }

    if (echo) {
      if (c == '\n')
        printw("\r\n");
      else
        addch(c);
    }

    return c;
  }

  std::variant<terminal::click_t, std::string> terminal::getline_or_click(bool echo, bool drop_on_click) {
    std::variant<terminal::click_t, std::string> ret;

    while (std::visit([&](auto&& x) -> bool {
      if constexpr (std::is_same_v<std::decay_t<decltype(x)>, terminal::click_t>) {
        // Reset the buffer if asked
        if (drop_on_click)
          _impl->wip.clear();
        ret = x;
        return false;
      }
      else if (x == '\n') {
        ret = std::move(_impl->wip);
        return false;
      }
      else {
        _impl->wip.push_back(x);
        return true;
      }
    }, get_next_event(echo)));

    return ret;
  }

  std::string terminal::getline(bool echo) {
    std::string ret;

    while (std::visit([&](auto&& x) -> bool {
      if constexpr (std::is_same_v<std::decay_t<decltype(x)>, std::string>) {
        // Reset the buffer if asked
        ret = std::move(x);
        return false;
      }
      else return true;
    }, getline_or_click(echo, false)));

    return ret;
  }

  void terminal::clear() {
    ::clear();
  }

  terminal::terminal() : _impl{std::make_unique<impl>()} {
    if (!::initscr() || ::noecho() || ::cbreak() || ::refresh())
      throw std::runtime_error("Could not create screen");

    // WHYYYYYYYYYYYYYYYY IS THIS NEEDED?
    //
    // If you know, please send me a message!
    keypad(stdscr, TRUE);


//    if (!(_impl->window = ::newwin(20, 20, 0, 0)) || ::wrefresh(_impl->window))
//      throw std::runtime_error("Could not create window");

//    ::box(_impl->window, 0, 0);
//    ::mvwprintw(_impl->window, 0, 0, "Hi!");

//    ::wrefresh(_impl->window);

    if (!::mousemask(BUTTON1_CLICKED | BUTTON3_CLICKED | REPORT_MOUSE_POSITION, nullptr))
      throw std::runtime_error("Could not set mousemask");
  }

  terminal::~terminal() {
    endwin();
  }
}
#endif
