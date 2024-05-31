
#ifndef ASIO_DETAIL_WAIT_HANDLER_HPP
#define ASIO_DETAIL_WAIT_HANDLER_HPP

#include "asio/detail/config.hpp"
#include "asio/detail/thread/fenced_block.hpp"
#include "asio/detail/memory/handler_alloc_helpers.hpp"
#include "asio/detail/scheduler/invoke/handler_invoke_helpers.hpp"
#include "asio/detail/memory/memory.hpp"
#include "asio/detail/scheduler/op/wait_op.hpp"
#include "asio/io_context.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename Handler>
class wait_handler : public wait_op
{
public:
  ASIO_DEFINE_HANDLER_PTR(wait_handler);

  wait_handler(Handler& h)
    : wait_op(&wait_handler::do_complete),
      handler_(ASIO_MOVE_CAST(Handler)(h))
  {
    handler_work<Handler>::start(handler_);
  }

  static void do_complete(void* owner, operation* base,
      const asio::error_code& /*ec*/,
      std::size_t /*bytes_transferred*/)
  {
    // Take ownership of the handler object.
    wait_handler* h(static_cast<wait_handler*>(base));
    ptr p = { asio::detail::addressof(h->handler_), h, h };
    handler_work<Handler> w(h->handler_);

    ASIO_HANDLER_COMPLETION((*h));

    // Make a copy of the handler so that the memory can be deallocated before
    // the upcall is made. Even if we're not about to make an upcall, a
    // sub-object of the handler may be the true owner of the memory associated
    // with the handler. Consequently, a local copy of the handler is required
    // to ensure that any owning sub-object remains valid until after we have
    // deallocated the memory here.
    detail::binder1<Handler, asio::error_code>
      handler(h->handler_, h->ec_);
    p.h = asio::detail::addressof(handler.handler_);
    p.reset();

    // Make the upcall if required.
    if (owner)
    {
      fenced_block b(fenced_block::half);
      
      w.complete(handler, handler.handler_);
    }
  }

private:
  Handler handler_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_WAIT_HANDLER_HPP
