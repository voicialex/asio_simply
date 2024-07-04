#ifndef ASIO_DETAIL_RESOLVE_OP_HPP
#define ASIO_DETAIL_RESOLVE_OP_HPP

#include "asio/detail/config.hpp"
#include "asio/error/error.hpp"
#include "asio/core/operation.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

class resolve_op : public operation
{
public:
  // The error code to be passed to the completion handler.
  asio::error_code ec_;

protected:
  resolve_op(func_type complete_func)
    : operation(complete_func)
  {
  }
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_RESOLVE_OP_HPP
