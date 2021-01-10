#include <cpplos/game.hpp>
#include <cpplos/common.hpp>

namespace cpplos {
  inline void handle_combos(cppthings::transaction& txn, board::cell side, board& board,
                            message_t::const_iterator front, message_t::const_iterator end) {
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

  /// By default we throw an error
  ///
  /// This is referenced by the later variant vistor,
  /// so this will provide specialisations for all commands
  template<typename T, typename>
  void game::handle_message_inner(T const&, message_t const&, board::cell) {
    throw move_error{"Invalid command given"};
  }

  template<>
  void game::handle_message_inner(commands::place const& x, message_t const& commands, board::cell side) {
    cppthings::transaction txn;

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

  template<>
  void game::handle_message_inner(commands::lift const& x, message_t const& commands, board::cell side) {
    cppthings::transaction txn;

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

  void game::handle_message(const message_t& commands, board::cell side) {
    if (commands.size() == 0)
      throw move_error{"No commands given in move"};

    std::visit([&](auto&& x) -> void {
      handle_message_inner<std::decay_t<decltype(x)>>(x, commands, side);
    }, commands.front());
  }
}
