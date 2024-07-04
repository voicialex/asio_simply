#ifndef ASIO_IP_BAD_ADDRESS_CAST_HPP
#define ASIO_IP_BAD_ADDRESS_CAST_HPP

#include "asio/detail/config.hpp"
#include <typeinfo>

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

/// Thrown to indicate a failed address conversion.
class bad_address_cast :
#if defined(ASIO_MSVC) && defined(_HAS_EXCEPTIONS) && !_HAS_EXCEPTIONS
  public std::exception
#else
  public std::bad_cast
#endif
{
public:
  /// Default constructor.
  bad_address_cast() {}

  /// Destructor.
  virtual ~bad_address_cast() ASIO_NOEXCEPT_OR_NOTHROW {}

  /// Get the message associated with the exception.
  virtual const char* what() const ASIO_NOEXCEPT_OR_NOTHROW
  {
    return "bad address cast";
  }
};

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_ADDRESS_HPP
