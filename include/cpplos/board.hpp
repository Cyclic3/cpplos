#pragma once

#include <cpplos/common.hpp>

#include <array>
#include <ios>
#include <string_view>
#include <vector>

#include <cstdint>

namespace cpplos {
  class board {
    friend std::ostream& operator<<(std::ostream& os, board const& b);

  public:
    enum cell : uint_fast8_t {
      Free = 0b0,
      DEPENDENT_MASK = 0b1111,


      Empty = 0x00,
      White = 0x10,
      Black = 0x20,
      INVALID = 0x30,
      SIDE_MASK = INVALID,
      SWAP_SIDE = 0x30,
    };

    struct coord_t {
      size_t x, y, z;
      constexpr size_t& operator[](size_t i) {
        switch (i) {
          case 0: return x; case 1: return y; case 2: return z;
          default: throw std::range_error{"Bad index for coords"};
        }
      }
      constexpr size_t const& operator[](size_t i) const {
        switch (i) {
          case 0: return x; case 1: return y; case 2: return z;
          default: throw std::range_error{"Bad index for coords"};
        }
      }
    };

  private:
    std::vector<std::vector<cell>> raw_board;

  private:
    inline size_t calculate_width(size_t height) const noexcept {
      return raw_board.size() - height;
    }
    inline size_t calculate_offset(coord_t const& c) const noexcept {
      size_t width = calculate_width(c[0]);
      return c[1] * width + c[2];
    }
    inline void construct_raw_board(size_t height) {
      for (size_t i = height; i > 0; --i)
        raw_board.emplace_back(i * i, cell::Empty);
    }

    inline bool can_get_square(coord_t const& top_left) const noexcept {
      auto width = calculate_width(top_left[0]);
      return top_left[1] < width && top_left[2] < width;
    }

    /// XXX: no bounds checks!
    std::array<cell*, 4> get_square(coord_t const& top_left) noexcept;
    /// XXX: no bounds checks!
    inline std::array<cell const*, 4> get_square(coord_t const& top_left) const noexcept;

    inline std::array<cell*, 4> get_base(coord_t const& position) {
      if (position[0] == 0)
        throw std::range_error("Cannot get base of bottom level!");

      return get_square({position[0] - 1, position[1], position[2]});
    }

  public:
    inline cell& operator[](coord_t const& coords) { return raw_board[coords[0]][calculate_offset(coords)]; }
    inline cell const& operator[](coord_t const& coords) const { return raw_board[coords[0]][calculate_offset(coords)]; }

    inline cell& at(coord_t const& c) {
      if (!check_coord_valid(c))
        throw std::range_error("Position was off the board");
      return (*this)[c];
    }
    inline cell const& at(coord_t const& c) const {
      if (!check_coord_valid(c))
        throw std::range_error("Position was off the board");
      return (*this)[c];
    }

    inline size_t get_height() const { return raw_board.size(); }
    inline bool check_coord_valid(coord_t const& c) const {
      if (c[0] >= raw_board.size())
        return false;
      size_t width = raw_board.size() - c[0];
      return c[1] < width && c[2] < width;
    }

    inline bool check_stable(coord_t const& position) {
      if (position[0] == 0)
        return true;

      for (auto& i : get_base(position))
        if (*i == cell::Empty)
          return false;

      return true;
    }

    inline cell get_apex() const noexcept {
      return raw_board.back()[0];
    }

    [[nodiscard]]
    cell* place(coord_t const& position, cell c);
    [[nodiscard]]
    cell* remove(coord_t const& position, cell c);
    bool check_combo(coord_t const& position, cell colour) const;
    void pretty_print(std::ostream&) const;

  public:
    inline board(size_t height = 4) {
      construct_raw_board(height);
    }

    board(std::string_view hjn);
  };

  std::ostream& operator<<(std::ostream& os, board::cell b);
  std::ostream& operator<<(std::ostream& os, board const& b);

  template<>
  inline board parse(std::string_view str) { return {str}; }

  template<>
  board::coord_t parse(std::string_view str);
  std::ostream& operator<<(std::ostream& os, board::coord_t const& c);
}
