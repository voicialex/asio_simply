#ifndef ASIO_DETAIL_TSS_PTR_HPP
#define ASIO_DETAIL_TSS_PTR_HPP

#include "asio/detail/config.hpp"


#if defined(ASIO_HAS_THREAD_KEYWORD_EXTENSION)
# include "asio/detail/base/impl_std/keyword_tss_ptr.hpp"
#elif defined(ASIO_HAS_PTHREADS)
# include "asio/detail/posix_tss_ptr.hpp"
#else
# error Only Windows and POSIX are supported!
#endif

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename T>
class tss_ptr
#if !defined(ASIO_HAS_THREADS)
  : public null_tss_ptr<T>
#elif defined(ASIO_HAS_THREAD_KEYWORD_EXTENSION)
  : public keyword_tss_ptr<T>
#elif defined(ASIO_HAS_PTHREADS)
  : public posix_tss_ptr<T>
#endif
{
public:
  void operator=(T* value)
  {
#if defined(ASIO_HAS_THREAD_KEYWORD_EXTENSION)
    keyword_tss_ptr<T>::operator=(value);
#elif defined(ASIO_HAS_PTHREADS)
    posix_tss_ptr<T>::operator=(value);
#endif
  }
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_TSS_PTR_HPP
