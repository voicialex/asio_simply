#ifndef ASIO_DETAIL_REACTIVE_WAIT_OP_HPP
#define ASIO_DETAIL_REACTIVE_WAIT_OP_HPP

#include "asio/detail/config.hpp"
#include "asio/detail/thread/fenced_block.hpp"
#include "asio/detail/memory/handler_alloc_helpers.hpp"
#include "asio/core/handler/handler_invoke_helpers.hpp"
#include "asio/detail/memory/memory.hpp"
#include "asio/detail/reactor/reactor_op.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename Handler>
class reactive_wait_op : public reactor_op
{
public:
  ASIO_DEFINE_HANDLER_PTR(reactive_wait_op);

  reactive_wait_op(Handler& handler)
    : reactor_op(&reactive_wait_op::do_perform,
        &reactive_wait_op::do_complete),
      handler_(static_cast<Handler&&>(handler))
  {
    handler_work<Handler>::start(handler_);
  }

  static status do_perform(reactor_op*)
  {
    return done;
  }

  static void do_complete(void* owner, operation* base,
      const asio::error_code& /*ec*/,
      std::size_t /*bytes_transferred*/)
  {
    // Take ownership of the handler object.
    reactive_wait_op* o(static_cast<reactive_wait_op*>(base));
    ptr p = { asio::detail::addressof(o->handler_), o, o };
    handler_work<Handler> w(o->handler_);

    ASIO_HANDLER_COMPLETION((*o));

    // Make a copy of the handler so that the memory can be deallocated before
    // the upcall is made. Even if we're not about to make an upcall, a
    // sub-object of the handler may be the true owner of the memory associated
    // with the handler. Consequently, a local copy of the handler is required
    // to ensure that any owning sub-object remains valid until after we have
    // deallocated the memory here.
    detail::binder1<Handler, asio::error_code>
      handler(o->handler_, o->ec_);
    p.h = asio::detail::addressof(handler.handler_);
    p.reset();

    // Make the upcall if required.
    if (owner)
    {
      fenced_block b(fenced_block::half);
      ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_));
      w.complete(handler, handler.handler_);
      ASIO_HANDLER_INVOCATION_END;
    }
  }

private:
  Handler handler_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_REACTIVE_WAIT_OP_HPP
