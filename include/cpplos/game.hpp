#pragma once

#include <cpplos/board.hpp>
#include <cpplos/commands.hpp>

#include <cppthings/defer.hpp>
#include <cppthings/sfinae.hpp>

#include <charconv>
#include <iostream>
#include <variant>

namespace cpplos {
  class game {
  public:
    board board;

  private:
    template<typename T, typename = typename std::enable_if_t<cppthings::is_in_variant_v<T, command_t>>>
    void handle_message_inner(T const&, message_t const& all_moves, board::cell side);

  public:
    board::cell winner() const noexcept {
      return board.get_apex();
    }

    void handle_message(const message_t& move, board::cell side);

    [[nodiscard]]
    inline bool try_place(board::coord_t position, board::cell colour) noexcept {
      // Does an implicit check of whether this is a legal position
      return board.place(position, colour) != nullptr;
    }

    [[nodiscard]]
    inline bool try_lift(board::coord_t from, board::coord_t to, board::cell colour) noexcept {
      // Lift *up*
      if (from[0] >= to[0])
        return false;

      // Does an implicit check of whether this is a legal position
      if (!board.remove(from, colour))
        return false;
      if (!board.place(to, colour)) {
        // revert change
        //
        // We cast away the nodiscard, as we are failing anyways, and this shouldn't fail
        (void)board.place(from, colour);
        return false;
      }
      return true;
    }
  };
}
