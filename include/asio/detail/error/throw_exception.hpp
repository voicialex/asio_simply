#ifndef ASIO_DETAIL_THROW_EXCEPTION_HPP
#define ASIO_DETAIL_THROW_EXCEPTION_HPP

#include "asio/detail/config.hpp"

namespace asio {
namespace detail {

// Declare the throw_exception function for all targets.
template <typename Exception>
void throw_exception(const Exception& e);

// Only define the throw_exception function when exceptions are enabled.
// Otherwise, it is up to the application to provide a definition of this
// function.
# if !defined(ASIO_NO_EXCEPTIONS)
template <typename Exception>
void throw_exception(const Exception& e)
{
  throw e;
}
# endif // !defined(ASIO_NO_EXCEPTIONS)

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_THROW_EXCEPTION_HPP
