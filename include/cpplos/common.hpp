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

  class token_iter : std::output_iterator_tag {
  private:
    std::string_view current;
    std::string_view remaining;
    bool is_str_lit;
    bool is_end;

  private:
    /// XXX: asumes remaining is of non-zero size
    void calculate_next_boundary() {
      if (remaining.front() == '"') {
        is_str_lit = true;
        remaining.remove_prefix(1);

        current = remaining;

        for (bool is_esc = false; ; remaining.remove_prefix(1)) {
          if (remaining.empty())
            throw parsing_error{"Unterminated string literal"};
          if (is_esc) {
            is_esc = false;
            continue;
          }
          switch (remaining.front()) {
            case '"': goto done;
            case '\\': is_esc = true;
            default: {}
          }
        }
        done:

        current.remove_suffix(remaining.size());
        remaining.remove_prefix(1);
      }
      else {
        is_str_lit = false;

        auto next_boundary = remaining.find(' ');
        if (next_boundary == remaining.npos)
          next_boundary = remaining.size();

        current = remaining.substr(0, next_boundary);
        remaining.remove_prefix(next_boundary);
      }
    }

  public:
    using value_type = std::string_view;
    using pointer = value_type const*;
    using reference = value_type const&;
    using iterator_category = std::output_iterator_tag;

    constexpr bool is_string_literal() const noexcept { return is_str_lit; }
    inline bool operator==(token_iter const& other) const noexcept {
      // This way works better for *this != end checks
      if (other.is_end)
        return is_end;
      else
        return !other.is_end && remaining == other.remaining;
    }
    inline bool operator!=(token_iter const& other) const noexcept {
      return !(*this == other);
    }

    /// XXX: doesn't check if it is an end iterator
    inline token_iter& operator++() {
      if (remaining.empty()) {
        is_end = true;
        return *this;
      }

      remaining.remove_prefix(1); // Remove space
      if (remaining.empty())
        throw parsing_error{"Trailing space in tokens"};

      calculate_next_boundary();

      return *this;
    }
    inline token_iter operator++(int) {
      token_iter ret = *this;
      ++(*this);
      return ret;
    }
    constexpr std::string_view& operator*() {
      return current;
    }
    constexpr std::string_view const& operator*() const {
      return current;
    }

  public:
    inline token_iter() : is_end{true} {}
    inline token_iter(std::string_view line) : remaining{line}, is_end{line.empty()} {
      if (!is_end)
        calculate_next_boundary();
    }

  public:
    static inline std::string encode_literal(std::string_view str) {
      std::string ret;
      ret.reserve(str.size() * 2 + 2);

      ret.push_back('"');

      for (auto i : str) {
        if (i == '"' || i == '\\')
          ret.push_back('\\');
        else if (i == '\n') {
          ret.append("\n");
          continue;
        }
        ret.push_back(i);
      }

      ret.push_back('"');

      return ret;
    }

    static inline std::string decode_literal(std::string_view str) {
      std::string ret;
      ret.reserve(str.size());

      bool is_esc = false;
      for (auto i : str) {
        if (is_esc) {
          is_esc = false;
          switch (i) {
            case 'n': ret.push_back(0x0a);
            default: ret.push_back(i);
          }
        }
        else if (i == '\\')
           is_esc = true;
        else
          ret.push_back(i);
      }

      return ret;
    }
  };

  /// Parses an object of type T from a string_view containing nothing else
  template<typename T>
  T parse(std::string_view str);

  /// Parses an object of type T,
  /// and modifies start to point to the area just after the parsed text
  template<typename T>
  T parse_streamed(token_iter& start, token_iter const& end);
}
