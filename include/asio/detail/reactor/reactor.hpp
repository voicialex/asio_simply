#ifndef ASIO_DETAIL_REACTOR_HPP
#define ASIO_DETAIL_REACTOR_HPP

#include "asio/detail/reactor/reactor_fwd.hpp"

#if defined(ASIO_HAS_EPOLL)
# include "asio/detail/reactor/epoll_reactor.hpp"
#else
# include "asio/detail/reactor/select_reactor.hpp"
#endif

#endif // ASIO_DETAIL_REACTOR_HPP
