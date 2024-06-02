
#ifndef ASIO_DETAIL_HANDLER_CONT_HELPERS_HPP
#define ASIO_DETAIL_HANDLER_CONT_HELPERS_HPP

#include "asio/detail/config.hpp"
#include "asio/detail/memory/memory.hpp"
#include "asio/detail/base/handler/handler_continuation_hook.hpp"

#include "asio/detail/push_options.hpp"

// Calls to asio_handler_is_continuation must be made from a namespace that
// does not contain overloads of this function. This namespace is defined here
// for that purpose.
namespace asio_handler_cont_helpers {

template <typename Context>
inline bool is_continuation(Context& context)
{
  using asio::asio_handler_is_continuation;
  return asio_handler_is_continuation(
      asio::detail::addressof(context));
}

} // namespace asio_handler_cont_helpers

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_HANDLER_CONT_HELPERS_HPP
