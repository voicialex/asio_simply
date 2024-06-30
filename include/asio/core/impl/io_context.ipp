#ifndef ASIO_IMPL_IO_CONTEXT_IPP
#define ASIO_IMPL_IO_CONTEXT_IPP

#include "asio/detail/config.hpp"
#include "asio/core/io_context.hpp"
#include "asio/detail/concurrency_hint.hpp"
#include "asio/detail/base/stdcpp/limits.hpp"
#include "asio/detail/base/scoped_ptr.hpp"
#include "asio/core/service_registry.hpp"
#include "asio/error/throw_error.hpp"

# include "asio/core/scheduler/scheduler.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

io_context::io_context()
  : impl_(add_impl(new impl_type(*this, ASIO_CONCURRENCY_HINT_DEFAULT)))
{
#ifdef ASIO_ENABLE_STUDY
  std::cout << "io_context " << std::endl;
#endif
}

io_context::io_context(int concurrency_hint)
  : impl_(add_impl(new impl_type(*this, concurrency_hint == 1
          ? ASIO_CONCURRENCY_HINT_1 : concurrency_hint)))
{
#ifdef ASIO_ENABLE_STUDY
  std::cout << "io_context " << "(concurrency_hint:" << concurrency_hint << ")" << std::endl;
#endif
}

io_context::impl_type& io_context::add_impl(io_context::impl_type* impl)
{
  asio::detail::scoped_ptr<impl_type> scoped_impl(impl);
  asio::add_service<impl_type>(*this, scoped_impl.get());
  return *scoped_impl.release();
}

io_context::~io_context()
{
#ifdef ASIO_ENABLE_STUDY
  std::cout << "~io_context " << std::endl;
#endif
}

io_context::count_type io_context::run()
{
  asio::error_code ec;
  count_type s = impl_.run(ec);
  asio::detail::throw_error(ec);
  return s;
}

io_context::count_type io_context::run_one()
{
  asio::error_code ec;
  count_type s = impl_.run_one(ec);
  asio::detail::throw_error(ec);
  return s;
}

io_context::count_type io_context::poll()
{
  asio::error_code ec;
  count_type s = impl_.poll(ec);
  asio::detail::throw_error(ec);
  return s;
}

io_context::count_type io_context::poll_one()
{
  asio::error_code ec;
  count_type s = impl_.poll_one(ec);
  asio::detail::throw_error(ec);
  return s;
}

void io_context::stop()
{
  impl_.stop();
}

bool io_context::stopped() const
{
  return impl_.stopped();
}

void io_context::restart()
{
  impl_.restart();
}

io_context::service::service(asio::io_context& owner)
  : execution_context::service(owner)
{
}

io_context::service::~service()
{
}

void io_context::service::shutdown()
{
}

void io_context::service::notify_fork(io_context::fork_event ev)
{
  (void)ev;
}


} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IMPL_IO_CONTEXT_IPP
