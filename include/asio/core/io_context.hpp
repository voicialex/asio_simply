#ifndef ASIO_IO_CONTEXT_HPP
#define ASIO_IO_CONTEXT_HPP

#include "asio/detail/config.hpp"
#include <cstddef>
#include <stdexcept>
#include <typeinfo>
#include "asio/core/executor/helper/async_result.hpp"
#include "asio/detail/noncopyable.hpp"
#include "asio/core/handler/wrapped_handler.hpp"
#include "asio/error/error_code.hpp"
#include "asio/core/execution_context.hpp"

# include "asio/detail/base/stdcpp/chrono.hpp"

# include "asio/detail/base/signal_init.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

namespace detail {

  typedef class scheduler io_context_impl;

} // namespace detail

/// Provides core I/O functionality.
class io_context
  : public execution_context
{
private:
  typedef detail::io_context_impl impl_type;

public:
  class executor_type;
  friend class executor_type;

  class service;

#if !defined(ASIO_NO_EXTENSIONS)
  class strand;
#endif // !defined(ASIO_NO_EXTENSIONS)

  typedef std::size_t count_type;

  ASIO_DECL io_context();

  ASIO_DECL explicit io_context(int concurrency_hint);

  ASIO_DECL ~io_context();

  executor_type get_executor() ASIO_NOEXCEPT;

  ASIO_DECL count_type run();

  template <typename Rep, typename Period>
  std::size_t run_for(const chrono::duration<Rep, Period>& rel_time);

  template <typename Clock, typename Duration>
  std::size_t run_until(const chrono::time_point<Clock, Duration>& abs_time);

  ASIO_DECL count_type run_one();

  template <typename Rep, typename Period>
  std::size_t run_one_for(const chrono::duration<Rep, Period>& rel_time);

  template <typename Clock, typename Duration>
  std::size_t run_one_until(
      const chrono::time_point<Clock, Duration>& abs_time);

  ASIO_DECL count_type poll();

  ASIO_DECL count_type poll_one();

  ASIO_DECL void stop();

  ASIO_DECL bool stopped() const;

  ASIO_DECL void restart();

private:
  // Helper function to add the implementation.
  ASIO_DECL impl_type& add_impl(impl_type* impl);

  // Backwards compatible overload for use with services derived from
  // io_context::service.
  template <typename Service>
  friend Service& use_service(io_context& ioc);

#if defined(__sun) || defined(__QNX__) || defined(__hpux) || defined(_AIX) \
  || defined(__osf__)
  detail::signal_init<> init_;
#endif

  // The implementation.
  impl_type& impl_;
};

/// Executor used to submit functions to an io_context.
class io_context::executor_type
{
public:
  /// Obtain the underlying execution context.
  io_context& context() const ASIO_NOEXCEPT;

  /// Inform the io_context that it has some outstanding work to do.
  void on_work_started() const ASIO_NOEXCEPT;

  /// Inform the io_context that some work is no longer outstanding.
  void on_work_finished() const ASIO_NOEXCEPT;

  /// Request the io_context to invoke the given function object.
  template <typename Function, typename Allocator>
  void dispatch(Function&& f, const Allocator& a) const;

  /// Request the io_context to invoke the given function object.
  template <typename Function, typename Allocator>
  void post(Function&& f, const Allocator& a) const;

  /// Request the io_context to invoke the given function object.
  template <typename Function, typename Allocator>
  void defer(Function&& f, const Allocator& a) const;

  bool running_in_this_thread() const ASIO_NOEXCEPT;

  friend bool operator==(const executor_type& a,
      const executor_type& b) ASIO_NOEXCEPT
  {
    return &a.io_context_ == &b.io_context_;
  }

  friend bool operator!=(const executor_type& a,
      const executor_type& b) ASIO_NOEXCEPT
  {
    return &a.io_context_ != &b.io_context_;
  }

private:
  friend class io_context;

  explicit executor_type(io_context& i) : io_context_(i) {}

  // The underlying io_context.
  io_context& io_context_;
};

/// Base class for all io_context services.
class io_context::service
  : public execution_context::service
{
public:
  asio::io_context& get_io_context();

private:
  /// Destroy all user-defined handler objects owned by the service.
  ASIO_DECL virtual void shutdown();

  ASIO_DECL virtual void notify_fork(
      execution_context::fork_event event);

protected:
  ASIO_DECL service(asio::io_context& owner);

  ASIO_DECL virtual ~service();
};

namespace detail {

// Special service base class to keep classes header-file only.
template <typename Type>
class service_base
  : public asio::io_context::service
{
public:
  static asio::detail::service_id<Type> id;

  // Constructor.
  service_base(asio::io_context& io_context)
    : asio::io_context::service(io_context)
  {
  }
};

template <typename Type>
asio::detail::service_id<Type> service_base<Type>::id;

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#include "asio/core/impl/io_context.hpp"
#if defined(ASIO_HEADER_ONLY)
# include "asio/core/impl/io_context.ipp"
#endif // defined(ASIO_HEADER_ONLY)

// If both io_context.hpp and strand.hpp have been included, automatically
// include the header file needed for the io_context::strand class.
#if !defined(ASIO_NO_EXTENSIONS)
# if defined(ASIO_STRAND_HPP)
#  include "asio/io_context_strand.hpp"
# endif // defined(ASIO_STRAND_HPP)
#endif // !defined(ASIO_NO_EXTENSIONS)

#endif // ASIO_IO_CONTEXT_HPP
