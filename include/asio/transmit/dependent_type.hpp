#ifndef ASIO_DETAIL_DEPENDENT_TYPE_HPP
#define ASIO_DETAIL_DEPENDENT_TYPE_HPP

#include "asio/detail/config.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename DependsOn, typename T>
struct dependent_type
{
  typedef T type;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_DEPENDENT_TYPE_HPP
