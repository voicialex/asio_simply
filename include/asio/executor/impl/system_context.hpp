#ifndef ASIO_IMPL_SYSTEM_CONTEXT_HPP
#define ASIO_IMPL_SYSTEM_CONTEXT_HPP

#include "asio/executor/system_executor.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

inline system_context::executor_type
system_context::get_executor() ASIO_NOEXCEPT
{
  return system_executor();
}

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IMPL_SYSTEM_CONTEXT_HPP
