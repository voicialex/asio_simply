#ifndef ASIO_DETAIL_REACTOR_FWD_HPP
#define ASIO_DETAIL_REACTOR_FWD_HPP


#include "asio/detail/config.hpp"

namespace asio {
namespace detail {

#if defined(ASIO_HAS_EPOLL)
typedef class epoll_reactor reactor;
#else
typedef class select_reactor reactor;
#endif

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_REACTOR_FWD_HPP
