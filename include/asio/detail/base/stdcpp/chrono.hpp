#ifndef ASIO_DETAIL_CHRONO_HPP
#define ASIO_DETAIL_CHRONO_HPP

#include "asio/detail/config.hpp"

#include <chrono>


namespace asio {
namespace chrono {

using std::chrono::duration;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::minutes;
using std::chrono::hours;
using std::chrono::time_point_cast;
#if defined(ASIO_HAS_STD_CHRONO_MONOTONIC_CLOCK)
typedef std::chrono::monotonic_clock steady_clock;
#else // defined(ASIO_HAS_STD_CHRONO_MONOTONIC_CLOCK)
using std::chrono::steady_clock;
#endif // defined(ASIO_HAS_STD_CHRONO_MONOTONIC_CLOCK)
using std::chrono::system_clock;
using std::chrono::high_resolution_clock;

} // namespace chrono
} // namespace asio

#endif // ASIO_DETAIL_CHRONO_HPP
