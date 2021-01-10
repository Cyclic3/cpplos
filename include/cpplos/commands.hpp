#pragma once

#include <cpplos/board.hpp>
#include <cpplos/common.hpp>

#include <algorithm>
#include <iostream>
#include <variant>

namespace cpplos::commands {
  struct command {
    /// Reads the command from a stream, excluding the type specifier
    virtual void read(token_iter& start, token_iter const& finish) = 0;
    /// Writes a command to a stream, excluding the type specifier
    virtual void write(std::ostream&) = 0;
    /// The character that uniquely identifies this command
    static constexpr char name = 0;
  };

#define CPPLOS_CMD(CHAR) \
  static constexpr char name = CHAR; \
  void read(token_iter& start, token_iter const& finish) override; \
  void write(std::ostream&) override;

  /// Commands that can be sent by a server on behalf of a player,
  /// or just by a player
  struct move_command : command {};
  /// Commands that can only be sent as a response to a player_command
  struct server_command : command {};

  struct place : move_command { CPPLOS_CMD('P'); board::coord_t target; };
  struct lift : move_command { CPPLOS_CMD('L'); board::coord_t from; board::coord_t to; };
  struct remove : move_command { CPPLOS_CMD('R'); board::coord_t target; };

  struct error : server_command { CPPLOS_CMD('?'); std::string message; };
  struct comment : server_command { CPPLOS_CMD('#'); std::string message; };

#undef CPPLOS_CMD

  template<typename T, typename = typename std::enable_if<std::is_base_of_v<command, T>>::type>
  inline std::ostream& operator<<(std::ostream& os, T& cmd) {
    os << T::name << ' ';
    cmd.write(os);
    return os;
  }

  using command_t = std::variant<place, lift, remove, error, comment>;
  using message_t = std::vector<command_t>;

  inline std::ostream& operator<<(std::ostream& os, command_t& cmd) {
    return std::visit([&](auto& x) -> std::ostream& { return os << x; }, cmd);
  }

  inline std::ostream& operator<<(std::ostream& os, message_t& move) {
    if (move.empty())
      return os;

    auto last = move.end() - 1;
    for (auto iter = move.begin(); iter != last; ++iter)
      os << *iter << ' ';

    return os << *last;
  }
}

namespace cpplos {
  using command_t = commands::command_t;
  using message_t = commands::message_t;
}
