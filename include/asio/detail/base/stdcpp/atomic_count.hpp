#ifndef ASIO_DETAIL_ATOMIC_COUNT_HPP
#define ASIO_DETAIL_ATOMIC_COUNT_HPP

#include "asio/detail/config.hpp"

#if !defined(ASIO_HAS_THREADS)
// Nothing to include.
#elif defined(ASIO_HAS_STD_ATOMIC)
# include <atomic>
#endif // defined(ASIO_HAS_STD_ATOMIC)

namespace asio {
namespace detail {

#if !defined(ASIO_HAS_THREADS)
typedef long atomic_count;
inline void increment(atomic_count& a, long b) { a += b; }
#elif defined(ASIO_HAS_STD_ATOMIC)
typedef std::atomic<long> atomic_count;
inline void increment(atomic_count& a, long b) { a += b; }
#endif // defined(ASIO_HAS_STD_ATOMIC)

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_ATOMIC_COUNT_HPP
