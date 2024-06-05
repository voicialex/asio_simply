#ifndef ASIO_DETAIL_SIGNAL_BLOCKER_HPP
#define ASIO_DETAIL_SIGNAL_BLOCKER_HPP

#include "asio/detail/config.hpp"

# include "asio/detail/base/posix/posix_signal_blocker.hpp"

namespace asio {
namespace detail {

typedef posix_signal_blocker signal_blocker;

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_SIGNAL_BLOCKER_HPP
