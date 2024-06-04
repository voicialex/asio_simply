#ifndef ASIO_DETAIL_NULL_TSS_PTR_HPP
#define ASIO_DETAIL_NULL_TSS_PTR_HPP

#include "asio/detail/config.hpp"

#if !defined(ASIO_HAS_THREADS)

#include "asio/detail/noncopyable.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename T>
class null_tss_ptr
  : private noncopyable
{
public:
  // Constructor.
  null_tss_ptr()
    : value_(0)
  {
  }

  // Destructor.
  ~null_tss_ptr()
  {
  }

  // Get the value.
  operator T*() const
  {
    return value_;
  }

  // Set the value.
  void operator=(T* value)
  {
    value_ = value;
  }

private:
  T* value_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // !defined(ASIO_HAS_THREADS)

#endif // ASIO_DETAIL_NULL_TSS_PTR_HPP
