#ifndef ASIO_IS_EXECUTOR_HPP
#define ASIO_IS_EXECUTOR_HPP

#include "asio/detail/config.hpp"
#include "asio/executor/impl/is_executor.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

/// The is_executor trait detects whether a type T meets the Executor type
/// requirements.
/**
 * Class template @c is_executor is a UnaryTypeTrait that is derived from @c
 * true_type if the type @c T meets the syntactic requirements for Executor,
 * otherwise @c false_type.
 */
template <typename T>
struct is_executor : asio::detail::is_executor<T>
{
};

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IS_EXECUTOR_HPP
