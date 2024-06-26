#ifndef ASIO_DETAIL_NONCOPYABLE_HPP
#define ASIO_DETAIL_NONCOPYABLE_HPP

#include "asio/detail/config.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

class noncopyable
{
protected:
  noncopyable() {}
  ~noncopyable() {}
private:
  noncopyable(const noncopyable&);
  const noncopyable& operator=(const noncopyable&);
};

} // namespace detail

using asio::detail::noncopyable;

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_NONCOPYABLE_HPP
