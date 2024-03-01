#ifndef ASIO_DETAIL_REACTOR_OP_HPP
#define ASIO_DETAIL_REACTOR_OP_HPP

#include "asio/detail/config.hpp"
#include "asio/detail/scheduler/op/operation.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

class reactor_op
  : public operation
{
public:
  // The error code to be passed to the completion handler.
  asio::error_code ec_;

  // The number of bytes transferred, to be passed to the completion handler.
  std::size_t bytes_transferred_;

  // Status returned by perform function. May be used to decide whether it is
  // worth performing more operations on the descriptor immediately.
  enum status { not_done, done, done_and_exhausted };

  // Perform the operation. Returns true if it is finished.
  status perform()
  {
    return perform_func_(this);
  }

protected:
  typedef status (*perform_func_type)(reactor_op*);

  reactor_op(perform_func_type perform_func, func_type complete_func)
    : operation(complete_func),
      bytes_transferred_(0),
      perform_func_(perform_func)
  {
  }

private:
  perform_func_type perform_func_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_REACTOR_OP_HPP
