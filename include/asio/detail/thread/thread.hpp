#ifndef ASIO_DETAIL_THREAD_HPP
#define ASIO_DETAIL_THREAD_HPP

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_PTHREADS)
# include "asio/detail/thread/impl/posix_thread.hpp"
#elif defined(ASIO_HAS_STD_THREAD)
# include "asio/detail/base/stdcpp/std_thread.hpp"
#else
# error Only Windows, POSIX and std::thread are supported!
#endif

namespace asio {
namespace detail {

#if defined(ASIO_HAS_PTHREADS)
typedef posix_thread thread;
#elif defined(ASIO_HAS_STD_THREAD)
typedef std_thread thread;
#endif

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_THREAD_HPP
