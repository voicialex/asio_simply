#ifndef ASIO_IP_IMPL_HOST_NAME_IPP
#define ASIO_IP_IMPL_HOST_NAME_IPP

#include "asio/detail/config.hpp"
#include "asio/network/socket_ops.hpp"
#include "asio/error/throw_error.hpp"
// #include "asio/detail/winsock_init.hpp"
#include "asio/ip/host_name.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

std::string host_name()
{
  char name[1024];
  asio::error_code ec;
  if (asio::detail::socket_ops::gethostname(name, sizeof(name), ec) != 0)
  {
    asio::detail::throw_error(ec);
    return std::string();
  }
  return std::string(name);
}

std::string host_name(asio::error_code& ec)
{
  char name[1024];
  if (asio::detail::socket_ops::gethostname(name, sizeof(name), ec) != 0)
    return std::string();
  return std::string(name);
}

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_IMPL_HOST_NAME_IPP
