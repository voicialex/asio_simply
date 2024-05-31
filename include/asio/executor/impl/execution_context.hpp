#ifndef ASIO_IMPL_EXECUTION_CONTEXT_HPP
#define ASIO_IMPL_EXECUTION_CONTEXT_HPP

// #include "asio/detail/handler_type_requirements.hpp"
#include "asio/detail/base/scoped_ptr.hpp"
#include "asio/detail/service_registry.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

template <typename Service>
inline Service& use_service(execution_context& e)
{
  // Check that Service meets the necessary type requirements.
  (void)static_cast<execution_context::service*>(static_cast<Service*>(0));

  return e.service_registry_->template use_service<Service>();
}

template <typename Service, typename... Args>
Service& make_service(execution_context& e, ASIO_MOVE_ARG(Args)... args)
{
  detail::scoped_ptr<Service> svc(
      new Service(e, ASIO_MOVE_CAST(Args)(args)...));
  e.service_registry_->template add_service<Service>(svc.get());
  Service& result = *svc;
  svc.release();
  return result;
}

template <typename Service>
inline void add_service(execution_context& e, Service* svc)
{
  // Check that Service meets the necessary type requirements.
  (void)static_cast<execution_context::service*>(static_cast<Service*>(0));

  e.service_registry_->template add_service<Service>(svc);
}

template <typename Service>
inline bool has_service(execution_context& e)
{
  // Check that Service meets the necessary type requirements.
  (void)static_cast<execution_context::service*>(static_cast<Service*>(0));

  return e.service_registry_->template has_service<Service>();
}

inline execution_context& execution_context::service::context()
{
  return owner_;
}

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IMPL_EXECUTION_CONTEXT_HPP
