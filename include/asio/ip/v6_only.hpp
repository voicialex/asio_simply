#ifndef ASIO_IP_V6_ONLY_HPP
#define ASIO_IP_V6_ONLY_HPP

#include "asio/detail/config.hpp"
#include "asio/network/socket_option.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

/// Socket option for determining whether an IPv6 socket supports IPv6
/// communication only.
/**
 * Implements the IPPROTO_IPV6/IP_V6ONLY socket option.
 *
 * @par Examples
 * Setting the option:
 * @code
 * asio::ip::tcp::socket socket(io_context); 
 * ...
 * asio::ip::v6_only option(true);
 * socket.set_option(option);
 * @endcode
 *
 * @par
 * Getting the current option value:
 * @code
 * asio::ip::tcp::socket socket(io_context); 
 * ...
 * asio::ip::v6_only option;
 * socket.get_option(option);
 * bool v6_only = option.value();
 * @endcode
 *
 * @par Concepts:
 * GettableSocketOption, SettableSocketOption.
 */
#if defined(GENERATING_DOCUMENTATION)
typedef implementation_defined v6_only;
#elif defined(IPV6_V6ONLY)
typedef asio::detail::socket_option::boolean<
    IPPROTO_IPV6, IPV6_V6ONLY> v6_only;
#else
typedef asio::detail::socket_option::boolean<
    asio::detail::custom_socket_option_level,
    asio::detail::always_fail_option> v6_only;
#endif

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_V6_ONLY_HPP
