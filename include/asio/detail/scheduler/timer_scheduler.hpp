#ifndef ASIO_DETAIL_TIMER_SCHEDULER_HPP
#define ASIO_DETAIL_TIMER_SCHEDULER_HPP

#include "asio/detail/config.hpp"
#include "asio/detail/scheduler/timer_scheduler_fwd.hpp"

#if defined(ASIO_HAS_EPOLL)
# include "asio/detail/reactor/epoll_reactor.hpp"
#elif defined(ASIO_HAS_KQUEUE)
# include "asio/detail/kqueue_reactor.hpp"
#elif defined(ASIO_HAS_DEV_POLL)
# include "asio/detail/dev_poll_reactor.hpp"
#else
# include "asio/detail/select_reactor.hpp"
#endif

#endif // ASIO_DETAIL_TIMER_SCHEDULER_HPP
