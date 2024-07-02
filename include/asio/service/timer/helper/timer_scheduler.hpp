#ifndef ASIO_DETAIL_TIMER_SCHEDULER_HPP
#define ASIO_DETAIL_TIMER_SCHEDULER_HPP

#include "asio/detail/config.hpp"
#include "asio/service/timer/helper/timer_scheduler_fwd.hpp"

#if defined(ASIO_HAS_EPOLL)
# include "asio/detail/reactor/epoll_reactor.hpp"
#else
# include "asio/detail/reactor/select_reactor.hpp"
#endif

#endif // ASIO_DETAIL_TIMER_SCHEDULER_HPP
