#ifndef ASIO_DETAIL_STD_FENCED_BLOCK_HPP
#define ASIO_DETAIL_STD_FENCED_BLOCK_HPP

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_STD_ATOMIC)

#include <atomic>
#include "asio/detail/noncopyable.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

class std_fenced_block
  : private noncopyable
{
public:
  enum half_t { half };
  enum full_t { full };

  // Constructor for a half fenced block.
  explicit std_fenced_block(half_t)
  {
  }

  // Constructor for a full fenced block.
  explicit std_fenced_block(full_t)
  {
    std::atomic_thread_fence(std::memory_order_acquire);
  }

  // Destructor.
  ~std_fenced_block()
  {
    std::atomic_thread_fence(std::memory_order_release);
  }
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_HAS_STD_ATOMIC)

#endif // ASIO_DETAIL_STD_FENCED_BLOCK_HPP
