
#ifndef ASIO_WAIT_TRAITS_HPP
#define ASIO_WAIT_TRAITS_HPP

#include "asio/detail/push_options.hpp"

namespace asio {

/// Wait traits suitable for use with the basic_waitable_timer class template.
template <typename Clock>
struct wait_traits
{
  /// Convert a clock duration into a duration used for waiting.
  /**
   * @returns @c d.
   */
  static typename Clock::duration to_wait_duration(
      const typename Clock::duration& d)
  {
    return d;
  }

  /// Convert a clock duration into a duration used for waiting.
  /**
   * @returns @c d.
   */
  static typename Clock::duration to_wait_duration(
      const typename Clock::time_point& t)
  {
    typename Clock::time_point now = Clock::now();
    if (now + (Clock::duration::max)() < t)
      return (Clock::duration::max)();
    if (now + (Clock::duration::min)() > t)
      return (Clock::duration::min)();
    return t - now;
  }
};

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_WAIT_TRAITS_HPP
