#ifndef ASIO_DETAIL_TIMER_QUEUE_BASE_HPP
#define ASIO_DETAIL_TIMER_QUEUE_BASE_HPP

#include "asio/detail/config.hpp"
#include "asio/detail/noncopyable.hpp"
#include "asio/detail/container/op_queue.hpp"
#include "asio/detail/scheduler/op/operation.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

class timer_queue_base
  : private noncopyable
{
public:
  // Constructor.
  timer_queue_base() : next_(0) {}

  // Destructor.
  virtual ~timer_queue_base() {}

  // Whether there are no timers in the queue.
  virtual bool empty() const = 0;

  // Get the time to wait until the next timer.
  virtual long wait_duration_msec(long max_duration) const = 0;

  // Get the time to wait until the next timer.
  virtual long wait_duration_usec(long max_duration) const = 0;

  // Dequeue all ready timers.
  virtual void get_ready_timers(op_queue<operation>& ops) = 0;

  // Dequeue all timers.
  virtual void get_all_timers(op_queue<operation>& ops) = 0;

private:
  friend class timer_queue_set;

  // Next timer queue in the set.
  timer_queue_base* next_;
};

template <typename Time_Traits>
class timer_queue;

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_TIMER_QUEUE_BASE_HPP
