#ifndef ASIO_DETAIL_HANDLER_WORK_HPP
#define ASIO_DETAIL_HANDLER_WORK_HPP

#include "asio/detail/config.hpp"
#include "asio/executor/associated_executor.hpp"
#include "asio/detail/scheduler/invoke/handler_invoke_helpers.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

// A helper class template to allow completion handlers to be dispatched
// through either the new executors framework or the old invocaton hook. The
// primary template uses the new executors framework.
template <typename Handler, typename Executor
    = typename associated_executor<Handler>::type>
class handler_work
{
public:
  explicit handler_work(Handler& handler) ASIO_NOEXCEPT
    : executor_(associated_executor<Handler>::get(handler))
  {
  }

  static void start(Handler& handler) ASIO_NOEXCEPT
  {
    Executor ex(associated_executor<Handler>::get(handler));
    ex.on_work_started();
  }

  ~handler_work()
  {
    executor_.on_work_finished();
  }

  template <typename Function>
  void complete(Function& function, Handler& handler)
  {
    executor_.dispatch(ASIO_MOVE_CAST(Function)(function),
        associated_allocator<Handler>::get(handler));
  }

private:
  // Disallow copying and assignment.
  handler_work(const handler_work&);
  handler_work& operator=(const handler_work&);

  typename associated_executor<Handler>::type executor_;
};

// This specialisation dispatches a handler through the old invocation hook.
// The specialisation is not strictly required for correctness, as the
// system_executor will dispatch through the hook anyway. However, by doing
// this we avoid an extra copy of the handler.
template <typename Handler>
class handler_work<Handler, system_executor>
{
public:
  explicit handler_work(Handler&) ASIO_NOEXCEPT {}
  static void start(Handler&) ASIO_NOEXCEPT {}
  ~handler_work() {}

  template <typename Function>
  void complete(Function& function, Handler& handler)
  {
    asio_handler_invoke_helpers::invoke(function, handler);
  }

private:
  // Disallow copying and assignment.
  handler_work(const handler_work&);
  handler_work& operator=(const handler_work&);
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_HANDLER_WORK_HPP
