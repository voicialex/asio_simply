#ifndef ASIO_DETAIL_GLOBAL_HPP
#define ASIO_DETAIL_GLOBAL_HPP

#include "asio/detail/config.hpp"

# include "asio/detail/base/posix/posix_global.hpp"

namespace asio {
namespace detail {

template <typename T>
inline T& global()
{
  return posix_global<T>();
}

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_GLOBAL_HPP
