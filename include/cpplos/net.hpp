#pragma once

#include <cpplos/commands.hpp>

#include <boost/asio/spawn.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace cpplos::net {
  class remote {
    boost::asio::ip::tcp::socket sock;

  public:
    void send_move(boost::asio::yield_context, move_t const& move);
    /// Tells the remote what side *they* should be
    void send_side(boost::asio::yield_context, board::cell side);

    move_t receive_move(boost::asio::yield_context);
    /// Tells you what the other side wants you to be
    board::cell receive_side(boost::asio::yield_context);

  public:
    inline remote(boost::asio::ip::tcp::socket&& sock) : sock{std::move(sock)} {}
  };
}
