#ifndef ASIO_DETAIL_WORK_DISPATCHER_HPP
#define ASIO_DETAIL_WORK_DISPATCHER_HPP

#include "asio/detail/config.hpp"
#include "asio/executor/helper/associated_executor.hpp"
#include "asio/detail/memory/associated_allocator.hpp"
#include "asio/executor/executor_work_guard.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename Handler>
class work_dispatcher
{
public:
  work_dispatcher(Handler& handler)
    : work_((get_associated_executor)(handler)),
      handler_(static_cast<Handler&&>(handler))
  {
  }

  work_dispatcher(const work_dispatcher& other)
    : work_(other.work_),
      handler_(other.handler_)
  {
  }

  work_dispatcher(work_dispatcher&& other)
    : work_(static_cast<executor_work_guard<
        typename associated_executor<Handler>::type>&&>(other.work_)),
      handler_(static_cast<Handler&&>(other.handler_))
  {
  }

  void operator()()
  {
    typename associated_allocator<Handler>::type alloc(
        (get_associated_allocator)(handler_));
    work_.get_executor().dispatch(
        static_cast<Handler&&>(handler_), alloc);
    work_.reset();
  }

private:
  executor_work_guard<typename associated_executor<Handler>::type> work_;
  Handler handler_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_WORK_DISPATCHER_HPP
