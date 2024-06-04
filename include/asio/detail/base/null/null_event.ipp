#ifndef ASIO_DETAIL_IMPL_NULL_EVENT_IPP
#define ASIO_DETAIL_IMPL_NULL_EVENT_IPP

#include "asio/detail/config.hpp"

# include <unistd.h>
# if !defined(__hpux) || defined(__SELECT)
#  include <sys/select.h>
# endif

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

void null_event::do_wait()
{
  ::pause();
}

void null_event::do_wait_for_usec(long usec)
{
  timeval tv;
  tv.tv_sec = usec / 1000000;
  tv.tv_usec = usec % 1000000;
  ::select(0, 0, 0, 0, &tv);
}

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_IMPL_NULL_EVENT_IPP
