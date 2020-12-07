#include <cpplos/net.hpp>

namespace cpplos::net {
  void remote::handshake(boost::asio::yield_context yield) {

    sock.async_write(yield);
  }
}
