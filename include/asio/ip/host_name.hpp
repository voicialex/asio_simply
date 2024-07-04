#ifndef ASIO_IP_HOST_NAME_HPP
#define ASIO_IP_HOST_NAME_HPP

#include "asio/detail/config.hpp"
#include <string>
#include "asio/error/error_code.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

/// Get the current host name.
ASIO_DECL std::string host_name();

/// Get the current host name.
ASIO_DECL std::string host_name(asio::error_code& ec);

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#if defined(ASIO_HEADER_ONLY)
# include "asio/ip/impl/host_name.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_IP_HOST_NAME_HPP
