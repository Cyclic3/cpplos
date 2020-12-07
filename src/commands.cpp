#include <cpplos/commands.hpp>

namespace cpplos::commands {
  void place::read(std::string_view& str) {
    auto end = str.find(' ');
    if (end == str.npos)
      end = str.size();
    else
      ++end;

    this->target = parse<board::coord_t>(str.substr(0, end));

    str.remove_prefix(end);
  }
  void place::write(std::ostream& os) { os << target; }

  void remove::read(std::string_view& str) {
    auto end = str.find(' ');
    if (end == str.npos)
      end = str.size();
    else
      ++end;

    this->target = parse<board::coord_t>(str.substr(0, end));

    str.remove_prefix(end);
  }
  void remove::write(std::ostream& os) { os << target; }

  void lift::read(std::string_view& str) {
    // Set end to just after the space, or fail if there's nothing there (because lift needs 2 params)
    auto end = str.find(' ');
    if (end == str.npos)
      throw parsing_error{"Lift needs two parameters"};
    else
      ++end;

    auto first_coords_part = str.substr(0, end);
    str.remove_prefix(end);

    end = str.find(' ');
    // Set end to just after the space, or the end of the string
    if (end == str.npos)
      end = str.size();
    else
      ++end;

    auto second_coords_part = str.substr(0, end);
    str.remove_prefix(end);

    this->from = parse<board::coord_t>(first_coords_part);
    this->to = parse<board::coord_t>(second_coords_part);
  }
  void lift::write(std::ostream& os) { os << from << ' ' << to; }
}

namespace cpplos {
#define CPPLOS_IMPL_PARSE(T) \
  template<> \
  inline commands::T parse_streamed(std::string_view& str) { \
    commands::T ret; \
    ret.read(str); \
    return ret; \
  }
  CPPLOS_IMPL_PARSE(place);
  CPPLOS_IMPL_PARSE(lift);
  CPPLOS_IMPL_PARSE(remove);

#undef CPPLOS_IMPL_PARSE


  template<>
  command_t parse_streamed(std::string_view& str) {
    if (str.size() < 2)
      throw parsing_error{"Command too short"};
    else if (str[1] != ' ')
      throw parsing_error{"Mangled command type"};

    auto t = str.front();
    str.remove_prefix(2);

    switch (t) {
#define CPPLOS_PARSE_ONE(T) \
    case commands::T::name: return parse_streamed<commands::T>(str);

      CPPLOS_PARSE_ONE(place);
      CPPLOS_PARSE_ONE(lift);
      CPPLOS_PARSE_ONE(remove);
      default: throw parsing_error{"Unknown command type"};

#undef CPPLOS_PARSE_ONE
    }
  }


  // This trivial implementation cannot be put before the impl of the command parser,
  // and so it can't go in the header =(
  template<>
  move_t parse(std::string_view str) {
    move_t ret;

    while (!str.empty())
      ret.emplace_back(parse_streamed<command_t>(str));

    return ret;
  }
}
