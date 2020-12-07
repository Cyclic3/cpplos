#pragma once

#include <stdexcept>

namespace cpplos {
  /// All types derived from public_error must be safe to send to a remote
  struct public_error : std::runtime_error {
    template<typename... Args>
    inline public_error(Args&&... args) : std::runtime_error{std::forward<Args>(args)...} {}
  };

  /// Represents a failure to parse some input
  struct parsing_error : public_error {
    template<typename... Args>
    inline parsing_error(Args&&... args) : public_error{std::forward<Args>(args)...} {}
  };

  /// Represents a failure to make a move
  struct move_error : public_error {
    template<typename... Args>
    inline move_error(Args&&... args) : public_error{std::forward<Args>(args)...} {}
  };

  /// Parses an object of type T from a string_view only containing nothing else
  template<typename T>
  T parse(std::string_view str);

  /// Parses an object of type T,
  /// and modifies str to point to the area just after the parsed text
  template<typename T>
  T parse_streamed(std::string_view& str);
}
