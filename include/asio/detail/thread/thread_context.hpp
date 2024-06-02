#ifndef ASIO_DETAIL_THREAD_CONTEXT_HPP
#define ASIO_DETAIL_THREAD_CONTEXT_HPP

#include <climits>
#include <cstddef>
#include "asio/detail/container/call_stack.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

class thread_info_base;

// Base class for things that manage threads (scheduler, win_iocp_io_context).
class thread_context
{
public:
  // Per-thread call stack to track the state of each thread in the context.
  typedef call_stack<thread_context, thread_info_base> thread_call_stack;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_THREAD_CONTEXT_HPP
