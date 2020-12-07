#if 0
#pragma once

#include <memory>
#include <string>
#include <variant>

namespace cpplos {
  class terminal;

  namespace detail {
    inline terminal make_terminal();
  }

  class terminal {
    friend terminal detail::make_terminal();

    struct impl;
  private:
    std::unique_ptr<impl> _impl;

  public:
    struct click_t {
      size_t x, y;
      enum { Left, Middle, Right } button;
      size_t count;
    };

  public:
    void clear();
    std::string getline(bool echo = true);

    std::variant<click_t, char> get_next_event(bool echo = true);
    std::variant<click_t, std::string> getline_or_click(bool echo = true, bool drop_on_click = true);
  private:
    terminal();
  public:
    ~terminal();
  };

  namespace detail {
    inline terminal make_terminal() { return {}; }
  }

  extern terminal term;
}
#endif
