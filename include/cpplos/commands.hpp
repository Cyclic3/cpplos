#pragma once

#include <cpplos/board.hpp>
#include <cpplos/common.hpp>

#include <iostream>
#include <variant>

namespace cpplos::commands {
  struct command {
    /// Reads the command from a stream, excluding the type specifier
    virtual void read(std::string_view&) = 0;
    /// Writes a command to a stream, excluding the type specifier
    virtual void write(std::ostream&) = 0;
    /// The character that uniquely identifies this command
    static constexpr char name = 0;
  };

#define CPPLOS_CMD(CHAR) \
  static constexpr char name = CHAR; \
  void read(std::string_view&) override; \
  void write(std::ostream&) override;

  /// Commands that can be sent by a server on behalf of a player,
  /// or just by a player
  struct player_command : command {};
  /// Commands that can only be sent as a response to a player_command
  struct server_command : command {};

  struct place : player_command { CPPLOS_CMD('P'); board::coord_t target; };
  struct lift : player_command { CPPLOS_CMD('L'); board::coord_t from; board::coord_t to; };
  struct remove : player_command { CPPLOS_CMD('R'); board::coord_t target; };

#undef CPPLOS_CMD

  template<typename T, typename = typename std::enable_if<std::is_base_of_v<command, T>>::type>
  inline std::ostream& operator<<(std::ostream& os, T& cmd) {
    os << T::name << ' ';
    cmd.write(os);
    return os;
  }

  using command_t = std::variant<place, lift, remove>;
  inline std::ostream& operator<<(std::ostream& os, command_t& cmd) {
    return std::visit([&](auto& x) -> std::ostream& { return os << x; }, cmd);
  }
}

namespace cpplos {
  using command_t = commands::command_t;
  using move_t = std::vector<command_t>;

  inline std::ostream& operator<<(std::ostream& os, move_t& move) {
    if (move.empty())
      return os;

    auto last = move.end() - 1;
    for (auto iter = move.begin(); iter != last; ++iter)
      os << *iter << ' ';

    return os << *last;
  }
}
