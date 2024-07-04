#ifndef ASIO_IP_RESOLVER_QUERY_BASE_HPP
#define ASIO_IP_RESOLVER_QUERY_BASE_HPP

#include "asio/detail/config.hpp"
#include "asio/ip/resolver_base.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

/// The resolver_query_base class is used as a base for the
/// basic_resolver_query class templates to provide a common place to define
/// the flag constants.
class resolver_query_base : public resolver_base
{
protected:
  /// Protected destructor to prevent deletion through this type.
  ~resolver_query_base()
  {
  }
};

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_RESOLVER_QUERY_BASE_HPP
