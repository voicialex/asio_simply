#ifndef ASIO_IMPL_DEFER_HPP
#define ASIO_IMPL_DEFER_HPP

#include "asio/detail/config.hpp"
#include "asio/detail/memory/associated_allocator.hpp"
#include "asio/core/executor/helper/associated_executor.hpp"
#include "asio/core/executor/submit/work_dispatcher.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

template <typename CompletionToken>
ASIO_INITFN_RESULT_TYPE(CompletionToken, void()) defer(
    CompletionToken&& token)
{
  typedef ASIO_HANDLER_TYPE(CompletionToken, void()) handler;

  async_completion<CompletionToken, void()> init(token);

  typename associated_executor<handler>::type ex(
      (get_associated_executor)(init.completion_handler));

  typename associated_allocator<handler>::type alloc(
      (get_associated_allocator)(init.completion_handler));

  ex.defer(static_cast<handler&&>(init.completion_handler), alloc);

  return init.result.get();
}

template <typename Executor, typename CompletionToken>
ASIO_INITFN_RESULT_TYPE(CompletionToken, void()) defer(
    const Executor& ex, CompletionToken&& token,
    typename enable_if<is_executor<Executor>::value>::type*)
{
  typedef ASIO_HANDLER_TYPE(CompletionToken, void()) handler;

  async_completion<CompletionToken, void()> init(token);

  typename associated_allocator<handler>::type alloc(
      (get_associated_allocator)(init.completion_handler));

  ex.defer(detail::work_dispatcher<handler>(init.completion_handler), alloc);

  return init.result.get();
}

template <typename ExecutionContext, typename CompletionToken>
inline ASIO_INITFN_RESULT_TYPE(CompletionToken, void()) defer(
    ExecutionContext& ctx, CompletionToken&& token,
    typename enable_if<is_convertible<
      ExecutionContext&, execution_context&>::value>::type*)
{
  return (defer)(ctx.get_executor(),
      static_cast<CompletionToken&&>(token));
}

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IMPL_DEFER_HPP
