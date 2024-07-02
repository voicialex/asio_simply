#ifndef ASIO_BASIC_WAITABLE_TIMER_HPP
#define ASIO_BASIC_WAITABLE_TIMER_HPP

#include "asio/detail/config.hpp"
#include <cstddef>
#include "asio/service/basic_io_object.hpp"
#include "asio/core/handler/handler_type_requirements.hpp"
#include "asio/error/throw_error.hpp"
#include "asio/error/error.hpp"
#include "asio/service/timer/helper/wait_traits.hpp"

# include <utility>

# include "asio/service/timer/helper/chrono_time_traits.hpp"
# include "asio/service/timer/deadline_timer_service.hpp"
# define ASIO_SVC_T \
    detail::deadline_timer_service< \
      detail::chrono_time_traits<Clock, WaitTraits> >

#include "asio/detail/push_options.hpp"

namespace asio {

#if !defined(ASIO_BASIC_WAITABLE_TIMER_FWD_DECL)
#define ASIO_BASIC_WAITABLE_TIMER_FWD_DECL

// Forward declaration with defaulted arguments.
template <typename Clock,
    typename WaitTraits = asio::wait_traits<Clock>
    ASIO_SVC_TPARAM_DEF2(= waitable_timer_service<Clock, WaitTraits>)>
class basic_waitable_timer;

#endif // !defined(ASIO_BASIC_WAITABLE_TIMER_FWD_DECL)

/// Provides waitable timer functionality.
template <typename Clock, typename WaitTraits ASIO_SVC_TPARAM>
class basic_waitable_timer
  : ASIO_SVC_ACCESS basic_io_object<ASIO_SVC_T>
{
public:
  /// The type of the executor associated with the object.
  typedef io_context::executor_type executor_type;

  /// The clock type.
  typedef Clock clock_type;

  /// The duration type of the clock.
  typedef typename clock_type::duration duration;

  /// The time point type of the clock.
  typedef typename clock_type::time_point time_point;

  /// The wait traits type.
  typedef WaitTraits traits_type;

  /// Constructor.
  explicit basic_waitable_timer(asio::io_context& io_context)
    : basic_io_object<ASIO_SVC_T>(io_context)
  {
  }

  /// Constructor to set a particular expiry time as an absolute time.
  basic_waitable_timer(asio::io_context& io_context,
      const time_point& expiry_time)
    : basic_io_object<ASIO_SVC_T>(io_context)
  {
    asio::error_code ec;
    this->get_service().expires_at(this->get_implementation(), expiry_time, ec);
    asio::detail::throw_error(ec, "expires_at");
  }

  /// Constructor to set a particular expiry time relative to now.
  basic_waitable_timer(asio::io_context& io_context,
      const duration& expiry_time)
    : basic_io_object<ASIO_SVC_T>(io_context)
  {
    asio::error_code ec;
    this->get_service().expires_after(
        this->get_implementation(), expiry_time, ec);
    asio::detail::throw_error(ec, "expires_after");
  }

  /// Move-construct a basic_waitable_timer from another.
  basic_waitable_timer(basic_waitable_timer&& other)
    : basic_io_object<ASIO_SVC_T>(std::move(other))
  {
  }

  /// Move-assign a basic_waitable_timer from another.
  basic_waitable_timer& operator=(basic_waitable_timer&& other)
  {
    basic_io_object<ASIO_SVC_T>::operator=(std::move(other));
    return *this;
  }

  /// Destroys the timer.
  ~basic_waitable_timer()
  {
  }

  /// Get the executor associated with the object.
  executor_type get_executor() ASIO_NOEXCEPT
  {
    return basic_io_object<ASIO_SVC_T>::get_executor();
  }

  /// Cancel any asynchronous operations that are waiting on the timer.
  std::size_t cancel()
  {
    asio::error_code ec;
    std::size_t s = this->get_service().cancel(this->get_implementation(), ec);
    asio::detail::throw_error(ec, "cancel");
    return s;
  }

  /// Cancels one asynchronous operation that is waiting on the timer.
  std::size_t cancel_one()
  {
    asio::error_code ec;
    std::size_t s = this->get_service().cancel_one(
        this->get_implementation(), ec);
    asio::detail::throw_error(ec, "cancel_one");
    return s;
  }

  /// Get the timer's expiry time as an absolute time.
  time_point expiry() const
  {
    return this->get_service().expiry(this->get_implementation());
  }

  /// Set the timer's expiry time as an absolute time.
  std::size_t expires_at(const time_point& expiry_time)
  {
    asio::error_code ec;
    std::size_t s = this->get_service().expires_at(
        this->get_implementation(), expiry_time, ec);
    asio::detail::throw_error(ec, "expires_at");
    return s;
  }

  /// Set the timer's expiry time relative to now.
  std::size_t expires_after(const duration& expiry_time)
  {
    asio::error_code ec;
    std::size_t s = this->get_service().expires_after(
        this->get_implementation(), expiry_time, ec);
    asio::detail::throw_error(ec, "expires_after");
    return s;
  }

  /// Perform a blocking wait on the timer.
  void wait()
  {
    asio::error_code ec;
    this->get_service().wait(this->get_implementation(), ec);
    asio::detail::throw_error(ec, "wait");
  }

  /// Perform a blocking wait on the timer.
  void wait(asio::error_code& ec)
  {
    this->get_service().wait(this->get_implementation(), ec);
  }

  /// Start an asynchronous wait on the timer.
  template <typename WaitHandler>
  ASIO_INITFN_RESULT_TYPE(WaitHandler,
      void (asio::error_code))
  async_wait(WaitHandler&& handler)
  {
    // If you get an error on the following line it means that your handler does
    // not meet the documented type requirements for a WaitHandler.
    ASIO_WAIT_HANDLER_CHECK(WaitHandler, handler) type_check;

    async_completion<WaitHandler,
      void (asio::error_code)> init(handler);

    this->get_service().async_wait(this->get_implementation(),
        init.completion_handler);

    return init.result.get();
  }

private:
  // Disallow copying and assignment.
  basic_waitable_timer(const basic_waitable_timer&) ASIO_DELETED;
  basic_waitable_timer& operator=(
      const basic_waitable_timer&) ASIO_DELETED;
};

} // namespace asio

#include "asio/detail/pop_options.hpp"

# undef ASIO_SVC_T

#endif // ASIO_BASIC_WAITABLE_TIMER_HPP
