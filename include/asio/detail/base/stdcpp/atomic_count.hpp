#ifndef ASIO_DETAIL_ATOMIC_COUNT_HPP
#define ASIO_DETAIL_ATOMIC_COUNT_HPP

#include "asio/detail/config.hpp"

# include <atomic>

namespace asio {
namespace detail {

typedef std::atomic<long> atomic_count;
inline void increment(atomic_count& a, long b) { a += b; }

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_ATOMIC_COUNT_HPP
