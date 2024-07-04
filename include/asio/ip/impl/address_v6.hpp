#ifndef ASIO_IP_IMPL_ADDRESS_V6_HPP
#define ASIO_IP_IMPL_ADDRESS_V6_HPP

#if !defined(ASIO_NO_IOSTREAM)

#include "asio/error/throw_error.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(
    std::basic_ostream<Elem, Traits>& os, const address_v6& addr)
{
  return os << addr.to_string().c_str();
}

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // !defined(ASIO_NO_IOSTREAM)

#endif // ASIO_IP_IMPL_ADDRESS_V6_HPP
