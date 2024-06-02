#ifndef ASIO_IMPL_EXECUTOR_IPP
#define ASIO_IMPL_EXECUTOR_IPP

#include "asio/detail/config.hpp"
#include "asio/executor/executor.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

bad_executor::bad_executor() ASIO_NOEXCEPT
{
}

const char* bad_executor::what() const ASIO_NOEXCEPT_OR_NOTHROW
{
  return "bad executor";
}

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IMPL_EXECUTOR_IPP
