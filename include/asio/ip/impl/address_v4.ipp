#ifndef ASIO_IP_IMPL_ADDRESS_V4_IPP
#define ASIO_IP_IMPL_ADDRESS_V4_IPP

#include "asio/detail/config.hpp"
#include <climits>
#include <limits>
#include <stdexcept>
#include "asio/error/error.hpp"
#include "asio/network/socket_ops.hpp"
#include "asio/error/throw_error.hpp"
#include "asio/error/throw_exception.hpp"
#include "asio/ip/address_v4.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

address_v4::address_v4(const address_v4::bytes_type& bytes)
{
#if UCHAR_MAX > 0xFF
  if (bytes[0] > 0xFF || bytes[1] > 0xFF
      || bytes[2] > 0xFF || bytes[3] > 0xFF)
  {
    std::out_of_range ex("address_v4 from bytes_type");
    asio::detail::throw_exception(ex);
  }
#endif // UCHAR_MAX > 0xFF

  using namespace std; // For memcpy.
  memcpy(&addr_.s_addr, bytes.data(), 4);
}

address_v4::address_v4(address_v4::uint_type addr)
{
  if ((std::numeric_limits<uint_type>::max)() > 0xFFFFFFFF)
  {
    std::out_of_range ex("address_v4 from unsigned integer");
    asio::detail::throw_exception(ex);
  }

  addr_.s_addr = asio::detail::socket_ops::host_to_network_long(
      static_cast<asio::detail::u_long_type>(addr));
}

address_v4::bytes_type address_v4::to_bytes() const ASIO_NOEXCEPT
{
  using namespace std; // For memcpy.
  bytes_type bytes;
#if defined(ASIO_HAS_STD_ARRAY)
  memcpy(bytes.data(), &addr_.s_addr, 4);
#else // defined(ASIO_HAS_STD_ARRAY)
  memcpy(bytes.elems, &addr_.s_addr, 4);
#endif // defined(ASIO_HAS_STD_ARRAY)
  return bytes;
}

address_v4::uint_type address_v4::to_uint() const ASIO_NOEXCEPT
{
  return asio::detail::socket_ops::network_to_host_long(addr_.s_addr);
}

std::string address_v4::to_string() const
{
  asio::error_code ec;
  char addr_str[asio::detail::max_addr_v4_str_len];
  const char* addr =
    asio::detail::socket_ops::inet_ntop(
        ASIO_OS_DEF(AF_INET), &addr_, addr_str,
        asio::detail::max_addr_v4_str_len, 0, ec);
  if (addr == 0)
    asio::detail::throw_error(ec);
  return addr;
}

bool address_v4::is_loopback() const ASIO_NOEXCEPT
{
  return (to_uint() & 0xFF000000) == 0x7F000000;
}

bool address_v4::is_unspecified() const ASIO_NOEXCEPT
{
  return to_uint() == 0;
}

bool address_v4::is_multicast() const ASIO_NOEXCEPT
{
  return (to_uint() & 0xF0000000) == 0xE0000000;
}

address_v4 make_address_v4(const char* str)
{
  asio::error_code ec;
  address_v4 addr = make_address_v4(str, ec);
  asio::detail::throw_error(ec);
  return addr;
}

address_v4 make_address_v4(const char* str,
    asio::error_code& ec) ASIO_NOEXCEPT
{
  address_v4::bytes_type bytes;
  if (asio::detail::socket_ops::inet_pton(
        ASIO_OS_DEF(AF_INET), str, &bytes, 0, ec) <= 0)
    return address_v4();
  return address_v4(bytes);
}

address_v4 make_address_v4(const std::string& str)
{
  return make_address_v4(str.c_str());
}

address_v4 make_address_v4(const std::string& str,
    asio::error_code& ec) ASIO_NOEXCEPT
{
  return make_address_v4(str.c_str(), ec);
}

#if defined(ASIO_HAS_STRING_VIEW)

address_v4 make_address_v4(string_view str)
{
  return make_address_v4(static_cast<std::string>(str));
}

address_v4 make_address_v4(string_view str,
    asio::error_code& ec) ASIO_NOEXCEPT
{
  return make_address_v4(static_cast<std::string>(str), ec);
}

#endif // defined(ASIO_HAS_STRING_VIEW)

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_IMPL_ADDRESS_V4_IPP
