#ifndef ASIO_DETAIL_STATIC_MUTEX_HPP
#define ASIO_DETAIL_STATIC_MUTEX_HPP

#include "asio/detail/config.hpp"


# include "asio/detail/base/posix/posix_static_mutex.hpp"


namespace asio {
namespace detail {

typedef posix_static_mutex static_mutex;
# define ASIO_STATIC_MUTEX_INIT ASIO_POSIX_STATIC_MUTEX_INIT

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_STATIC_MUTEX_HPP
