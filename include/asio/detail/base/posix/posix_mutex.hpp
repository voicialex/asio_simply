#ifndef ASIO_DETAIL_POSIX_MUTEX_HPP
#define ASIO_DETAIL_POSIX_MUTEX_HPP

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_PTHREADS)

#include <pthread.h>
#include "asio/detail/noncopyable.hpp"
#include "asio/detail/base/scoped_lock.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

class posix_event;

class posix_mutex
  : private noncopyable
{
public:
  typedef asio::detail::scoped_lock<posix_mutex> scoped_lock;

  // Constructor.
  ASIO_DECL posix_mutex();

  // Destructor.
  ~posix_mutex()
  {
    ::pthread_mutex_destroy(&mutex_); // Ignore EBUSY.
  }

  // Lock the mutex.
  void lock()
  {
    (void)::pthread_mutex_lock(&mutex_); // Ignore EINVAL.
  }

  // Unlock the mutex.
  void unlock()
  {
    (void)::pthread_mutex_unlock(&mutex_); // Ignore EINVAL.
  }

private:
  friend class posix_event;
  ::pthread_mutex_t mutex_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#if defined(ASIO_HEADER_ONLY)
# include "asio/detail/base/posix/posix_mutex.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // defined(ASIO_HAS_PTHREADS)

#endif // ASIO_DETAIL_POSIX_MUTEX_HPP
