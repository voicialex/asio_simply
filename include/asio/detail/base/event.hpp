#ifndef ASIO_DETAIL_EVENT_HPP
#define ASIO_DETAIL_EVENT_HPP

#include "asio/detail/config.hpp"

#if !defined(ASIO_HAS_THREADS)
# include "asio/detail/base/null_event.hpp"
#elif defined(ASIO_HAS_PTHREADS)
# include "asio/detail/base/impl/posix_event.hpp"
#elif defined(ASIO_HAS_STD_MUTEX_AND_CONDVAR)
# include "asio/detail/std_event.hpp"
#else
# error Only Windows, POSIX and std::condition_variable are supported!
#endif

namespace asio {
namespace detail {

#if !defined(ASIO_HAS_THREADS)
typedef null_event event;
#elif defined(ASIO_HAS_PTHREADS)
typedef posix_event event;
#elif defined(ASIO_HAS_STD_MUTEX_AND_CONDVAR)
typedef std_event event;
#endif

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_EVENT_HPP
