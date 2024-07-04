#ifndef ASIO_IP_IMPL_BASIC_ENDPOINT_HPP
#define ASIO_IP_IMPL_BASIC_ENDPOINT_HPP

#if !defined(ASIO_NO_IOSTREAM)

#include "asio/error/throw_error.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

template <typename Elem, typename Traits, typename InternetProtocol>
std::basic_ostream<Elem, Traits>& operator<<(
    std::basic_ostream<Elem, Traits>& os,
    const basic_endpoint<InternetProtocol>& endpoint)
{
  asio::ip::detail::endpoint tmp_ep(endpoint.address(), endpoint.port());
  return os << tmp_ep.to_string().c_str();
}

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // !defined(ASIO_NO_IOSTREAM)

#endif // ASIO_IP_IMPL_BASIC_ENDPOINT_HPP
