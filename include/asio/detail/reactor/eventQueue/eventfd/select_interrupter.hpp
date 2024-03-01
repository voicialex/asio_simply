#ifndef ASIO_DETAIL_SELECT_INTERRUPTER_HPP
#define ASIO_DETAIL_SELECT_INTERRUPTER_HPP

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_EVENTFD)
# include "asio/detail/reactor/eventQueue/eventfd/eventfd_select_interrupter.hpp"
#else
# include "asio/detail/pipe_select_interrupter.hpp"
#endif

namespace asio {
namespace detail {

#if defined(ASIO_HAS_EVENTFD)
typedef eventfd_select_interrupter select_interrupter;
#else
typedef pipe_select_interrupter select_interrupter;
#endif

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_SELECT_INTERRUPTER_HPP
