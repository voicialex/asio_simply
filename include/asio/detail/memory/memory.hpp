#ifndef ASIO_DETAIL_MEMORY_HPP
#define ASIO_DETAIL_MEMORY_HPP

#include "asio/detail/config.hpp"
#include <memory>

namespace asio {
namespace detail {

using std::shared_ptr;
using std::weak_ptr;

using std::addressof;

} // namespace detail

#if defined(ASIO_HAS_CXX11_ALLOCATORS)
using std::allocator_arg_t;
# define ASIO_USES_ALLOCATOR(t) \
  namespace std { \
    template <typename Allocator> \
    struct uses_allocator<t, Allocator> : true_type {}; \
  } \
  /**/
# define ASIO_REBIND_ALLOC(alloc, t) \
  typename std::allocator_traits<alloc>::template rebind_alloc<t>
  /**/
#else // defined(ASIO_HAS_CXX11_ALLOCATORS)
struct allocator_arg_t {};
# define ASIO_USES_ALLOCATOR(t)
# define ASIO_REBIND_ALLOC(alloc, t) \
  typename alloc::template rebind<t>::other
  /**/
#endif // defined(ASIO_HAS_CXX11_ALLOCATORS)

} // namespace asio

#endif // ASIO_DETAIL_MEMORY_HPP
