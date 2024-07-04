#ifndef ASIO_STEADY_TIMER_HPP
#define ASIO_STEADY_TIMER_HPP

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_CHRONO)

#include "asio/service/timer/basic_waitable_timer.hpp"
#include "asio/detail/base/stdcpp/chrono.hpp"

namespace asio {

/// Typedef for a timer based on the steady clock.
/**
 * This typedef uses the C++11 @c &lt;chrono&gt; standard library facility, if
 * available. Otherwise, it may use the Boost.Chrono library. To explicitly
 * utilise Boost.Chrono, use the basic_waitable_timer template directly:
 * @code
 * typedef basic_waitable_timer<boost::chrono::steady_clock> timer;
 * @endcode
 */
typedef basic_waitable_timer<chrono::steady_clock> steady_timer;

} // namespace asio

#endif // defined(ASIO_HAS_CHRONO) || defined(GENERATING_DOCUMENTATION)

#endif // ASIO_STEADY_TIMER_HPP
