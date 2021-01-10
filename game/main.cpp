#include <cpplos/game.hpp>
#include <cpplos/rating.hpp>

#include <iostream>
#include <sstream>

std::vector<cpplos::commands::comment> comment_board(cpplos::board const& board) {
  std::stringstream ss;
  board.pretty_print(ss);
  std::vector<cpplos::commands::comment> ret;
  std::string str;
  while (std::getline(ss, str)) {
    ret.emplace_back();
    ret.back().message = str;
  }
  return ret;
}

int main() {
  cpplos::game game;

  std::string line;

  cpplos::board::cell side = cpplos::board::cell::White;
//  if (std::getline(std::cin, line) && !line.empty()) {
//    game.board = cpplos::board{line};
//    while (true) {
//      std::cout << "Player: " << std::flush;
//      if (!std::getline(std::cin, line))
//        throw std::runtime_error("Couldn't determine player");
//      if (line.size() != 1)
//        continue;
//      switch (line.front()) {
//        case 'W': side = cpplos::board::cell::White; goto ready;
//        case 'B': side = cpplos::board::cell::Black; goto ready;
//        default: continue;
//      }
//    }
//  }

  ready:

  while (true) {
    std::cout << "\033c" << std::flush;

    if (game.winner()) {
      std::cout << game.winner() << " WINS!!!" << std::endl;
      goto done;
    }

//    for (cpplos::commands::comment& i : comment_board(game.board)) {
//      cpplos::command_t cmd{std::in_place_type<cpplos::commands::comment>, std::move(i)};
//      std::cout << cmd << std::endl;
//    }

//    std::cout << "# \"HJN: " << game.board << '"' << std::endl;

    game.board.pretty_print(std::cout);
    std::cout << "HJN: " << game.board << std::endl;

    std::string line;

    retry:
    try {
      std::cout << side << "> " << std::flush;
      if (!std::getline(std::cin, line) || line.empty())
        goto done;
      auto move = cpplos::parse<cpplos::message_t>(line);
      game.handle_message(move, side);
    }
    catch (cpplos::parsing_error& exn) {
      cpplos::commands::error error;
      error.message = "PARSER: ";
      error.message += exn.what();
      std::cout << error << std::endl;
      goto retry;
    }
    catch (cpplos::move_error& exn) {
      cpplos::commands::error error;
      error.message = "MOVE: ";
      error.message += exn.what();
      std::cout << error << std::endl;
      goto retry;
    }

    reinterpret_cast<std::underlying_type_t<decltype(side)>&>(side) ^= cpplos::board::cell::SWAP_SIDE;
  }

  done:
  return 0;
}
