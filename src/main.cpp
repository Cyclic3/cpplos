#include <cpplos/game.hpp>

#include <iostream>

int main() {
  cpplos::game game;

  std::string line;

  cpplos::board::cell side = cpplos::board::cell::White;

  std::cout << "HJN: " << std::flush;
  if (std::getline(std::cin, line) && !line.empty()) {
    game.board = cpplos::board{line};
    while (true) {
      std::cout << "Player: " << std::flush;
      if (!std::getline(std::cin, line))
        throw std::runtime_error("Couldn't determine player");
      if (line.size() != 1)
        continue;
      switch (line.front()) {
        case 'W': side = cpplos::board::cell::White; goto ready;
        case 'B': side = cpplos::board::cell::Black; goto ready;
        default: continue;
      }
    }
  }

  ready:

  while (true) {
    std::cout << "\033c" << std::flush;

    if (game.winner()) {
      std::cout << game.winner() << " WINS!!!" << std::endl;
      goto done;
    }

    game.board.pretty_print(std::cout);
    std::cout << "HJN: " << game.board << std::endl;

    std::string line;

    retry:
    try {
      std::cout << side << "> " << std::flush;
      if (!std::getline(std::cin, line) || line.empty())
        goto done;
      auto move = cpplos::parse<cpplos::move_t>(line);
      game.do_move(move, side);
    }
    catch (cpplos::parsing_error& exn) {
      std::cout << "PARSER: " << exn.what() << std::endl;
      goto retry;
    }
    catch (cpplos::move_error& exn) {
      std::cout << "MOVE: " << exn.what() << std::endl;
      goto retry;
    }

    reinterpret_cast<std::underlying_type_t<decltype(side)>&>(side) ^= cpplos::board::cell::SWAP_SIDE;
  }

  done:
  return 0;
}
