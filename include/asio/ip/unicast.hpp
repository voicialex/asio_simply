#ifndef ASIO_IP_UNICAST_HPP
#define ASIO_IP_UNICAST_HPP

#include "asio/detail/config.hpp"
#include <cstddef>
#include "asio/ip/detail/socket_option.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {
namespace unicast {

/// Socket option for time-to-live associated with outgoing unicast packets.
/**
 * Implements the IPPROTO_IP/IP_UNICAST_TTL socket option.
 *
 * @par Examples
 * Setting the option:
 * @code
 * asio::ip::udp::socket socket(io_context); 
 * ...
 * asio::ip::unicast::hops option(4);
 * socket.set_option(option);
 * @endcode
 *
 * @par
 * Getting the current option value:
 * @code
 * asio::ip::udp::socket socket(io_context); 
 * ...
 * asio::ip::unicast::hops option;
 * socket.get_option(option);
 * int ttl = option.value();
 * @endcode
 *
 * @par Concepts:
 * GettableSocketOption, SettableSocketOption.
 */
typedef asio::ip::detail::socket_option::unicast_hops<
  ASIO_OS_DEF(IPPROTO_IP),
  ASIO_OS_DEF(IP_TTL),
  ASIO_OS_DEF(IPPROTO_IPV6),
  ASIO_OS_DEF(IPV6_UNICAST_HOPS)> hops;

} // namespace unicast
} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_UNICAST_HPP
