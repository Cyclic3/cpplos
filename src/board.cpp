#include <cpplos/board.hpp>
#include <cpplos/common.hpp>

#include <iostream>
#include <charconv>

namespace cpplos {
  /// XXX: no bounds checks!
  std::array<board::cell*, 4> board::get_square(board::coord_t const& top_left) noexcept {
    auto level_n = top_left[0];
    auto& level = raw_board[level_n];
    size_t width = calculate_width(level_n);
    size_t base_point = top_left[1] * width + top_left[2];

    return {
      &level[base_point],
      &level[base_point + 1],
      &level[base_point + width],
      &level[base_point + width + 1],
    };
  }

  /// XXX: no bounds checks!
  std::array<board::cell const*, 4> board::get_square(coord_t const& top_left) const noexcept {
    auto level_n = top_left[0];
    auto& level = raw_board[level_n];
    size_t width = calculate_width(level_n);
    size_t base_point = top_left[1] * width + top_left[2];

    return {
      &level[base_point],
      &level[base_point + 1],
      &level[base_point + width],
      &level[base_point + width + 1],
    };
  }

  std::ostream& operator<<(std::ostream& os, board::cell b) {
    constexpr std::array<char, 4> chars{'E', 'W', 'B', '?'};
    os << chars.at(static_cast<size_t>((b & board::cell::SIDE_MASK) >> 4));
    return os;
  }

  std::ostream& operator<<(std::ostream& os, board const& b) {
    size_t run_length = 0;
    board::cell last_cell = board::cell::INVALID;

    for (auto& level : b.raw_board) {
      for (auto& cell : level) {
        if (last_cell == cell) {
          ++run_length;
          continue;
        }

        // Print previous cell
        if (run_length > 1)
          os << run_length;
        if (last_cell != board::cell::INVALID)
          os << last_cell;

        last_cell = cell;
        run_length = 1;
      }
    }

    // Print previous cell
    if (run_length > 1)
      os << run_length;

    if (last_cell != board::cell::INVALID)
      os << last_cell;

    return os;
  }

  bool board::check_combo(coord_t const& position, cell colour) const {
    if (position[0] == raw_board.size() - 1)
      return false;

    auto level = raw_board[position[0]];
    size_t width = calculate_width(position[0]);

    // y:
    {
      size_t begin = position[1] * width;
      for (size_t i = 0; i < width; ++i)
        if (!(level[begin + i] & colour))
          goto z;
      return true;
    }
    z:
    {
      size_t begin = position[2];
      for (size_t i = 0; i < width; ++i)
        if (!(level[begin + i * width] & colour))
          goto square_1;
      return true;
    }
    square_1:
    {
      if (can_get_square(position)) {
        for (auto* i : get_square(position))
          if (!(*i & colour))
            goto square_2;
        return true;
      }
    }
    square_2:
    if (position[1] != 0) {
      coord_t new_pos = {position[0], position[1], position[2] - 1};
      if (can_get_square(new_pos)) {
        for (auto* i : get_square(new_pos))
          if (!(*i & colour))
            goto square_3;
        return true;
      }
    }
    square_3:
    if (position[2] != 0) {
      coord_t new_pos = {position[0], position[1] - 1, position[2]};
      if (can_get_square(new_pos)) {
        for (auto* i : get_square(new_pos))
          if (!(*i & colour))
            goto square_4;
        return true;
      }
    }
    square_4:
    if (position[1] != 0 && position[2] != 0) {
      coord_t new_pos = {position[0], position[1] - 1, position[2] - 1};
      if (can_get_square(new_pos)) {
        for (auto* i : get_square(new_pos))
          if (!(*i & colour))
            goto finish;
        return true;
      }
    }

    finish:
    return false;
  }

  void board::pretty_print(std::ostream& os) const {
    size_t width = raw_board.size();
    for (size_t level = 0; level < raw_board.size(); ++level, --width) {
      for (size_t i = 0; i < width; ++i) {
        for (size_t j = 0; j < width; ++j) {
          auto& target = (*this)[{level, i, j}];
          if (target == board::Empty)
            os << '*';
          else
            os << target;
        }
        os << std::endl;
      }
      os << std::endl;
    }
  }

  template<>
  board::coord_t parse(std::string_view str) {
    board::coord_t ret;
    auto start = &str.front();
    for (size_t i = 0; true; ++i) {
       auto res = std::from_chars(start, &str.back() + 1, ret[i]);
       if (res.ec != std::errc{})
         throw parsing_error{"Coord integer parsing error"};

       if (i == 2)
         return ret;

       start = res.ptr + 1;
       if (start > &str.back())
         throw parsing_error{"Coords to short"};
    }
  }

  board::cell* board::place(coord_t const& position, cell c) {
    if (!check_coord_valid(position))
      return nullptr;

    auto& target = (*this)[position];

    if (target != cell::Empty)
      return nullptr;

    if (position[0] == 0) {
      target = c;
      return &target;
    }

    auto base = get_base(position);
    // We have to iterate twice, as we can't add dependencies until we know everything works
    for (auto& i : base) {
      if (*i == cell::Empty)
        return nullptr;
    }
    for (auto* i : base) {
      ++*reinterpret_cast<std::underlying_type_t<cell>*>(i); // Add a dependent
    }

    target = c;
    return &target;
  }

  board::cell* board::remove(coord_t const& position, cell colour) {
    if (!check_coord_valid(position))
      return nullptr;

    auto& target = (*this)[position];

    if ((target & cell::DEPENDENT_MASK) || (colour && !(colour & target)))
      return nullptr;

    target = cell::Empty;

    if (position[0] != 0)
      for (auto& i : get_base(position))
         --*reinterpret_cast<std::underlying_type_t<cell>*>(i); // Remove a dependent

    return &target;
  }

  board::board(std::string_view hjn) {
    std::vector<cell> decoded;
    size_t rep = 0;
    for (auto& i : hjn) {
      switch (i) {
        case 'W': decoded.insert(decoded.end(), rep ? rep/10 : 1, cell::White); rep = 0; break;
        case 'B': decoded.insert(decoded.end(), rep ? rep/10 : 1, cell::Black); rep = 0; break;
        case 'E': decoded.insert(decoded.end(), rep ? rep/10 : 1, cell::Empty); rep = 0; break;
        case '9': ++rep;
        case '8': ++rep;
        case '7': ++rep;
        case '6': ++rep;
        case '5': ++rep;
        case '4': ++rep;
        case '3': ++rep;
        case '2': ++rep;
        case '1': ++rep;
        case '0': rep *= 10; break;
        default: throw parsing_error("Bad char in HJN");
      }
    }

    size_t height = 0;
    size_t i = 0;
    for (; i < decoded.size(); i += height*height)
      ++height;
    if (i != decoded.size()) {
      throw parsing_error("Bad decoded length in HJN");
    }

    construct_raw_board(height);

    // We could do the following, but that doesn't track dependents
    /*
    auto iter = decoded.begin();
    for (auto& level : raw_board)
      for (auto& cell: level)
        cell = *iter++;
    */

    auto iter = decoded.begin();
    size_t level = 0;
    size_t width = height;

    for (; level < height; ++level, --width) {
      for (size_t i = 0; i < width; ++i) {
        for (size_t j = 0; j < width; ++j) {
          auto& c = *iter++;
          if (c != cell::Empty)
            if (!place({level, i, j}, c))
              throw parsing_error{"Could not place ball in postion from HJN"};
        }
      }
    }

    return;
  }

  std::ostream& operator<<(std::ostream& os, board::coord_t const& c) {
    return os << c[0] << ',' << c[1] << ',' << c[2];
  }
}
