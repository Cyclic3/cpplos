#include <cpplos/commands.hpp>

namespace cpplos::commands {
  void place::read(token_iter& start, token_iter const& finish) {
    if (start == finish)
      throw parsing_error{"Too few parameters for place"};

    this->target = parse<board::coord_t>(*start++);
  }
  void place::write(std::ostream& os) { os << target; }

  void remove::read(token_iter& start, token_iter const& finish) {
    if (start == finish)
      throw parsing_error{"Too few parameters for remove"};

    this->target = parse<board::coord_t>(*start++);
  }
  void remove::write(std::ostream& os) { os << target; }

  void lift::read(token_iter& start, token_iter const& finish) {
    if (start == finish)
      throw parsing_error{"Too few parameters for lift"};

    this->from = parse<board::coord_t>(*start++);

    if (start == finish)
      throw parsing_error{"Too few parameters for lift"};

    this->to = parse<board::coord_t>(*start++);
  }
  void lift::write(std::ostream& os) { os << from << ' ' << to; }

  void error::read(token_iter& start, token_iter const& finish) {
    if (start == finish)
      return this->message.clear();

    if (start.is_string_literal())
      this->message = token_iter::decode_literal(*start);
    else
      this->message = *start;
  }
  void error::write(std::ostream& os) {
    if (!message.empty())
      os << token_iter::encode_literal(message);
  }

  void comment::read(token_iter& start, token_iter const& finish) {
    if (start == finish)
      return this->message.clear();

    if (start.is_string_literal())
      this->message = token_iter::decode_literal(*start);
    else
      this->message = *start;
  }
  void comment::write(std::ostream& os) {
    if (!message.empty())
      os << token_iter::encode_literal(message);
  }
}

namespace cpplos {
#define CPPLOS_IMPL_PARSE(T) \
  template<> \
  inline commands::T parse_streamed(token_iter& start, token_iter const& finish) { \
    commands::T ret; \
    ret.read(start, finish); \
    return ret; \
  }
  CPPLOS_IMPL_PARSE(place);
  CPPLOS_IMPL_PARSE(lift);
  CPPLOS_IMPL_PARSE(remove);
  CPPLOS_IMPL_PARSE(error);
  CPPLOS_IMPL_PARSE(comment);

#undef CPPLOS_IMPL_PARSE


  template<>
  commands::command_t parse_streamed(token_iter& start, token_iter const& finish) {
    if (start == finish)
      throw parsing_error{"Empty command given"};

    auto str = *start++;

    if (str.size() != 1)
      throw parsing_error{"Mangled command type"};

    switch (str.front()) {
#define CPPLOS_PARSE_ONE(T) \
    case commands::T::name: return parse_streamed<commands::T>(start, finish);

      CPPLOS_PARSE_ONE(place);
      CPPLOS_PARSE_ONE(lift);
      CPPLOS_PARSE_ONE(remove);
      CPPLOS_PARSE_ONE(error);
      CPPLOS_PARSE_ONE(comment);
      default: throw parsing_error{"Unknown command type"};

#undef CPPLOS_PARSE_ONE
    }
  }


  // This trivial implementation cannot be put before the impl of the command parser,
  // and so it can't go in the header =(
  template<>
  message_t parse(std::string_view str) {
    message_t ret;

    token_iter start{str}, end;

    while (start != end)
      ret.emplace_back(parse_streamed<command_t>(start, end));

    return ret;
  }
}
