#ifndef ASIO_DETAIL_TIMER_SCHEDULER_FWD_HPP
#define ASIO_DETAIL_TIMER_SCHEDULER_FWD_HPP

#include "asio/detail/config.hpp"

namespace asio {
namespace detail {

#if defined(ASIO_HAS_EPOLL)
typedef class epoll_reactor timer_scheduler;
#elif defined(ASIO_HAS_KQUEUE)
typedef class kqueue_reactor timer_scheduler;
#elif defined(ASIO_HAS_DEV_POLL)
typedef class dev_poll_reactor timer_scheduler;
#else
typedef class select_reactor timer_scheduler;
#endif

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_TIMER_SCHEDULER_FWD_HPP
