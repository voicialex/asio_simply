#ifndef ASIO_DETAIL_RESOLVER_ENDPOINT_OP_HPP
#define ASIO_DETAIL_RESOLVER_ENDPOINT_OP_HPP

#include "asio/detail/config.hpp"
#include "asio/error/error.hpp"
#include "asio/core/io_context.hpp"
#include "asio/ip/basic_resolver_results.hpp"
#include "asio/core/handler/bind_handler.hpp"
#include "asio/detail/thread/fenced_block.hpp"
#include "asio/detail/memory/handler_alloc_helpers.hpp"
#include "asio/core/handler/handler_invoke_helpers.hpp"
#include "asio/detail/memory/memory.hpp"
#include "asio/network/resolve_op.hpp"
#include "asio/network/socket_ops.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename Protocol, typename Handler>
class resolve_endpoint_op : public resolve_op
{
public:
  ASIO_DEFINE_HANDLER_PTR(resolve_endpoint_op);

  typedef typename Protocol::endpoint endpoint_type;
  typedef asio::ip::basic_resolver_results<Protocol> results_type;

  resolve_endpoint_op(socket_ops::weak_cancel_token_type cancel_token,
      const endpoint_type& endpoint, io_context_impl& ioc, Handler& handler)
    : resolve_op(&resolve_endpoint_op::do_complete),
      cancel_token_(cancel_token),
      endpoint_(endpoint),
      io_context_impl_(ioc),
      handler_(static_cast<Handler&&>(handler))
  {
    handler_work<Handler>::start(handler_);
  }

  static void do_complete(void* owner, operation* base,
      const asio::error_code& /*ec*/,
      std::size_t /*bytes_transferred*/)
  {
    // Take ownership of the operation object.
    resolve_endpoint_op* o(static_cast<resolve_endpoint_op*>(base));
    ptr p = { asio::detail::addressof(o->handler_), o, o };
    handler_work<Handler> w(o->handler_);

    if (owner && owner != &o->io_context_impl_)
    {
      // The operation is being run on the worker io_context. Time to perform
      // the resolver operation.
    
      // Perform the blocking endpoint resolution operation.
      char host_name[NI_MAXHOST];
      char service_name[NI_MAXSERV];
      socket_ops::background_getnameinfo(o->cancel_token_, o->endpoint_.data(),
          o->endpoint_.size(), host_name, NI_MAXHOST, service_name, NI_MAXSERV,
          o->endpoint_.protocol().type(), o->ec_);
      o->results_ = results_type::create(o->endpoint_, host_name, service_name);

      // Pass operation back to main io_context for completion.
      o->io_context_impl_.post_deferred_completion(o);
      p.v = p.p = 0;
    }
    else
    {
      // The operation has been returned to the main io_context. The completion
      // handler is ready to be delivered.

      ASIO_HANDLER_COMPLETION((*o));

      // Make a copy of the handler so that the memory can be deallocated
      // before the upcall is made. Even if we're not about to make an upcall,
      // a sub-object of the handler may be the true owner of the memory
      // associated with the handler. Consequently, a local copy of the handler
      // is required to ensure that any owning sub-object remains valid until
      // after we have deallocated the memory here.
      detail::binder2<Handler, asio::error_code, results_type>
        handler(o->handler_, o->ec_, o->results_);
      p.h = asio::detail::addressof(handler.handler_);
      p.reset();

      if (owner)
      {
        fenced_block b(fenced_block::half);
        ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_, "..."));
        w.complete(handler, handler.handler_);
        ASIO_HANDLER_INVOCATION_END;
      }
    }
  }

private:
  socket_ops::weak_cancel_token_type cancel_token_;
  endpoint_type endpoint_;
  io_context_impl& io_context_impl_;
  Handler handler_;
  results_type results_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_RESOLVER_ENDPOINT_OP_HPP
