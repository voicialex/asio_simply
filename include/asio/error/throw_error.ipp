#ifndef ASIO_DETAIL_IMPL_THROW_ERROR_IPP
#define ASIO_DETAIL_IMPL_THROW_ERROR_IPP

#include "asio/detail/config.hpp"
#include "asio/error/throw_error.hpp"
#include "asio/error/throw_exception.hpp"
#include "asio/error/system_error.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

void do_throw_error(const asio::error_code& err)
{
  asio::system_error e(err);
  asio::detail::throw_exception(e);
}

void do_throw_error(const asio::error_code& err, const char* location)
{
  // boostify: non-boost code starts here
  asio::system_error e(err, location);
  asio::detail::throw_exception(e);
  // boostify: non-boost code ends here
}

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_IMPL_THROW_ERROR_IPP
