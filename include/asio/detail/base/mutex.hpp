#ifndef ASIO_DETAIL_MUTEX_HPP
#define ASIO_DETAIL_MUTEX_HPP

#include "asio/detail/config.hpp"


# include "asio/detail/base/posix/posix_mutex.hpp"

namespace asio {
namespace detail {

typedef posix_mutex mutex;

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_MUTEX_HPP
