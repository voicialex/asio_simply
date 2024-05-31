#ifndef ASIO_DETAIL_SIGNAL_INIT_HPP
#define ASIO_DETAIL_SIGNAL_INIT_HPP

#include "asio/detail/config.hpp"

#include <csignal>

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <int Signal = SIGPIPE>
class signal_init
{
public:
  // Constructor.
  signal_init()
  {
    std::signal(Signal, SIG_IGN);
  }
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_SIGNAL_INIT_HPP
