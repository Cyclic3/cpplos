#include <cpplos/game.hpp>
#include <cpplos/common.hpp>

namespace cpplos {
  inline void handle_combos(cppthings::transaction& txn, board::cell side, board& board,
                            move_t::const_iterator front, move_t::const_iterator end) {
    auto n_removes = end - front;

    // You have to remove 1 or 2 balls
    if (n_removes < 1 || n_removes > 2)
      throw move_error{"You must remove either one or two balls with a combo"};

    for (auto iter = front; iter != end; ++iter) {
      std::visit([&](auto&& x) {
        if constexpr (!std::is_same_v<std::decay_t<decltype(x)>, commands::remove>)
          throw move_error{"You can only chain remove commands for a combo"};
        // Has to be in an else for constexpr reasons
        else {
          if (!board.remove(x.target, side))
            throw move_error{"Could not remove from given position"};
          txn.add([&board, side, x] { (void) board.place(x.target, side); });
        }
      }, *iter);
    }
  }

  void game::do_move(move_t const& commands, board::cell side) {
    // Because I don't trust myself not to screw up
    cppthings::transaction txn;

    if (commands.size() == 0)
      throw move_error{"No commands given in move"};

    std::visit([&](auto&& x) -> void {
      using T = std::decay_t<decltype(x)>;
      if constexpr (std::is_same_v<T, commands::place>) {
        if (!try_place(x.target, side))
          throw move_error{"Could not place in given position"};

        // If there are no more combos, we stop
        if (!board.check_combo(x.target, side)) {
          // No removing without a combo
          if (commands.size() != 1)
            throw move_error{"Cannot remove without a combo"};

          return txn.commit();
        }

        // We have to track the place now
        txn.add([&]{ (void) board.remove(x.target, side); });

        handle_combos(txn, side, board, commands.begin() + 1, commands.end());

        return txn.commit();
      }
      else if constexpr (std::is_same_v<T, commands::lift>) {
        if (!try_lift(x.from, x.to, side))
          throw move_error{"Could lift from/to given position"};

        // If there are no more combos, we stop
        //
        // We only need to check for combos where we target
        if (!board.check_combo(x.to, side)) {
          // No removing without a combo
          if (commands.size() != 1)
            throw move_error{"Cannot remove without a combo"};

          return txn.commit();
        }

        // We have to track the lift now
        txn.add([&]{ (void) board.remove(x.to, side); (void) board.place(x.from, side); });

        handle_combos(txn, side, board, commands.begin() + 1, commands.end());

        return txn.commit();
      }
      else {
        throw move_error{"Invalid move"};
      }
    }, commands.front());
  }
}
