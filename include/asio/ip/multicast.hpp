#ifndef ASIO_IP_MULTICAST_HPP
#define ASIO_IP_MULTICAST_HPP

#include "asio/detail/config.hpp"
#include <cstddef>
#include "asio/ip/detail/socket_option.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {
namespace multicast {

/// Socket option to join a multicast group on a specified interface.
/**
 * Implements the IPPROTO_IP/IP_ADD_MEMBERSHIP socket option.
 *
 * @par Examples
 * Setting the option to join a multicast group:
 * @code
 * asio::ip::udp::socket socket(io_context); 
 * ...
 * asio::ip::address multicast_address =
 *   asio::ip::address::from_string("225.0.0.1");
 * asio::ip::multicast::join_group option(multicast_address);
 * socket.set_option(option);
 * @endcode
 *
 * @par Concepts:
 * SettableSocketOption.
 */
typedef asio::ip::detail::socket_option::multicast_request<
  ASIO_OS_DEF(IPPROTO_IP),
  ASIO_OS_DEF(IP_ADD_MEMBERSHIP),
  ASIO_OS_DEF(IPPROTO_IPV6),
  ASIO_OS_DEF(IPV6_JOIN_GROUP)> join_group;

/// Socket option to leave a multicast group on a specified interface.
/**
 * Implements the IPPROTO_IP/IP_DROP_MEMBERSHIP socket option.
 *
 * @par Examples
 * Setting the option to leave a multicast group:
 * @code
 * asio::ip::udp::socket socket(io_context); 
 * ...
 * asio::ip::address multicast_address =
 *   asio::ip::address::from_string("225.0.0.1");
 * asio::ip::multicast::leave_group option(multicast_address);
 * socket.set_option(option);
 * @endcode
 *
 * @par Concepts:
 * SettableSocketOption.
 */
typedef asio::ip::detail::socket_option::multicast_request<
  ASIO_OS_DEF(IPPROTO_IP),
  ASIO_OS_DEF(IP_DROP_MEMBERSHIP),
  ASIO_OS_DEF(IPPROTO_IPV6),
  ASIO_OS_DEF(IPV6_LEAVE_GROUP)> leave_group;

/// Socket option for local interface to use for outgoing multicast packets.
/**
 * Implements the IPPROTO_IP/IP_MULTICAST_IF socket option.
 *
 * @par Examples
 * Setting the option:
 * @code
 * asio::ip::udp::socket socket(io_context); 
 * ...
 * asio::ip::address_v4 local_interface =
 *   asio::ip::address_v4::from_string("1.2.3.4");
 * asio::ip::multicast::outbound_interface option(local_interface);
 * socket.set_option(option);
 * @endcode
 *
 * @par Concepts:
 * SettableSocketOption.
 */
typedef asio::ip::detail::socket_option::network_interface<
  ASIO_OS_DEF(IPPROTO_IP),
  ASIO_OS_DEF(IP_MULTICAST_IF),
  ASIO_OS_DEF(IPPROTO_IPV6),
  ASIO_OS_DEF(IPV6_MULTICAST_IF)> outbound_interface;

/// Socket option for time-to-live associated with outgoing multicast packets.
/**
 * Implements the IPPROTO_IP/IP_MULTICAST_TTL socket option.
 *
 * @par Examples
 * Setting the option:
 * @code
 * asio::ip::udp::socket socket(io_context); 
 * ...
 * asio::ip::multicast::hops option(4);
 * socket.set_option(option);
 * @endcode
 *
 * @par
 * Getting the current option value:
 * @code
 * asio::ip::udp::socket socket(io_context); 
 * ...
 * asio::ip::multicast::hops option;
 * socket.get_option(option);
 * int ttl = option.value();
 * @endcode
 *
 * @par Concepts:
 * GettableSocketOption, SettableSocketOption.
 */
typedef asio::ip::detail::socket_option::multicast_hops<
  ASIO_OS_DEF(IPPROTO_IP),
  ASIO_OS_DEF(IP_MULTICAST_TTL),
  ASIO_OS_DEF(IPPROTO_IPV6),
  ASIO_OS_DEF(IPV6_MULTICAST_HOPS)> hops;

/// Socket option determining whether outgoing multicast packets will be
/// received on the same socket if it is a member of the multicast group.
/**
 * Implements the IPPROTO_IP/IP_MULTICAST_LOOP socket option.
 *
 * @par Examples
 * Setting the option:
 * @code
 * asio::ip::udp::socket socket(io_context); 
 * ...
 * asio::ip::multicast::enable_loopback option(true);
 * socket.set_option(option);
 * @endcode
 *
 * @par
 * Getting the current option value:
 * @code
 * asio::ip::udp::socket socket(io_context); 
 * ...
 * asio::ip::multicast::enable_loopback option;
 * socket.get_option(option);
 * bool is_set = option.value();
 * @endcode
 *
 * @par Concepts:
 * GettableSocketOption, SettableSocketOption.
 */
typedef asio::ip::detail::socket_option::multicast_enable_loopback<
  ASIO_OS_DEF(IPPROTO_IP),
  ASIO_OS_DEF(IP_MULTICAST_LOOP),
  ASIO_OS_DEF(IPPROTO_IPV6),
  ASIO_OS_DEF(IPV6_MULTICAST_LOOP)> enable_loopback;

} // namespace multicast
} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_MULTICAST_HPP
