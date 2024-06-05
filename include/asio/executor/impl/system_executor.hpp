#ifndef ASIO_IMPL_SYSTEM_EXECUTOR_HPP
#define ASIO_IMPL_SYSTEM_EXECUTOR_HPP

#include "asio/executor/executor_op.hpp"
#include "asio/detail/base/global.hpp"
#include "asio/detail/memory/recycling_allocator.hpp"
#include "asio/detail/base/stdcpp/type_traits.hpp"
#include "asio/core/system_context.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

inline system_context& system_executor::context() const ASIO_NOEXCEPT
{
  return detail::global<system_context>();
}

template <typename Function, typename Allocator>
void system_executor::dispatch(
    Function&& f, const Allocator&) const
{
  typename decay<Function>::type tmp(static_cast<Function&&>(f));
  asio_handler_invoke_helpers::invoke(tmp, tmp);
}

template <typename Function, typename Allocator>
void system_executor::post(
    Function&& f, const Allocator& a) const
{
  typedef typename decay<Function>::type function_type;

  system_context& ctx = detail::global<system_context>();

  // Allocate and construct an operation to wrap the function.
  typedef detail::executor_op<function_type, Allocator> op;
  typename op::ptr p = { detail::addressof(a), op::ptr::allocate(a), 0 };
  p.p = new (p.v) op(static_cast<Function&&>(f), a);

  ctx.scheduler_.post_immediate_completion(p.p, false);
  p.v = p.p = 0;
}

template <typename Function, typename Allocator>
void system_executor::defer(
    Function&& f, const Allocator& a) const
{
  typedef typename decay<Function>::type function_type;

  system_context& ctx = detail::global<system_context>();

  // Allocate and construct an operation to wrap the function.
  typedef detail::executor_op<function_type, Allocator> op;
  typename op::ptr p = { detail::addressof(a), op::ptr::allocate(a), 0 };
  p.p = new (p.v) op(static_cast<Function&&>(f), a);

  ctx.scheduler_.post_immediate_completion(p.p, true);
  p.v = p.p = 0;
}

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IMPL_SYSTEM_EXECUTOR_HPP
