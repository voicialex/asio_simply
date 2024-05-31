#ifndef ASIO_DETAIL_TSS_PTR_HPP
#define ASIO_DETAIL_TSS_PTR_HPP

#include "asio/detail/config.hpp"
#include "asio/detail/base/posix/posix_tss_ptr.hpp"
#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename T>
class tss_ptr
  : public posix_tss_ptr<T>
{
public:
  void operator=(T* value)
  {
    posix_tss_ptr<T>::operator=(value);
  }
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_TSS_PTR_HPP
