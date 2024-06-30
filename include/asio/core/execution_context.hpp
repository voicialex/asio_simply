#ifndef ASIO_EXECUTION_CONTEXT_HPP
#define ASIO_EXECUTION_CONTEXT_HPP

#include "asio/detail/config.hpp"
#include <cstddef>
#include <stdexcept>
#include <typeinfo>
#include "asio/detail/noncopyable.hpp"
// #include "asio/detail/variadic_templates.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

class execution_context;
class io_context;

template <typename Service> Service& use_service(execution_context&);
template <typename Service> Service& use_service(io_context&);
template <typename Service> void add_service(execution_context&, Service*);
template <typename Service> bool has_service(execution_context&);

namespace detail { class service_registry; }

/// A context for function object execution.
class execution_context
  : private noncopyable
{
public:
  class id;
  class service;

protected:
  ASIO_DECL execution_context();

  ASIO_DECL ~execution_context();

  /// Shuts down all services in the context.
  ASIO_DECL void shutdown();

  /// Destroys all services in the context.
  ASIO_DECL void destroy();

public:
  /// Fork-related event notifications.
  enum fork_event
  {
    /// Notify the context that the process is about to fork.
    fork_prepare,

    /// Notify the context that the process has forked and is the parent.
    fork_parent,

    /// Notify the context that the process has forked and is the child.
    fork_child
  };

  /// Notify the execution_context of a fork-related event.
  ASIO_DECL void notify_fork(fork_event event);

  /// Obtain the service object corresponding to the given type.
  template <typename Service>
  friend Service& use_service(execution_context& e);

  /// Obtain the service object corresponding to the given type.
  template <typename Service>
  friend Service& use_service(io_context& ioc);

  template <typename Service, typename... Args>
  friend Service& make_service(execution_context& e,
      Args&&... args);

  /// (Deprecated: Use make_service().) Add a service object to the
  /// execution_context.
  template <typename Service>
  friend void add_service(execution_context& e, Service* svc);

  /// Determine if an execution_context contains a specified service type.
  template <typename Service>
  friend bool has_service(execution_context& e);

private:
  // The service registry.
  asio::detail::service_registry* service_registry_;
};

/// Class used to uniquely identify a service.
class execution_context::id
  : private noncopyable
{
public:
  /// Constructor.
  id() {}
};

/// Base class for all io_context services.
class execution_context::service
  : private noncopyable
{
public:
  /// Get the context object that owns the service.
  execution_context& context();

protected:
  ASIO_DECL service(execution_context& owner);

  ASIO_DECL virtual ~service();

private:
  /// Destroy all user-defined handler objects owned by the service.
  virtual void shutdown() = 0;

  /// Handle notification of a fork-related event to perform any necessary
  /// housekeeping.
  /**
   * This function is not a pure virtual so that services only have to
   * implement it if necessary. The default implementation does nothing.
   */
  ASIO_DECL virtual void notify_fork(
      execution_context::fork_event event);

  friend class asio::detail::service_registry;
  struct key
  {
    key() : type_info_(0), id_(0) {}
    const std::type_info* type_info_;
    const execution_context::id* id_;
  } key_;

  execution_context& owner_;
  service* next_;
};

/// Exception thrown when trying to add a duplicate service to an
/// execution_context.
class service_already_exists
  : public std::logic_error
{
public:
  ASIO_DECL service_already_exists();
};

/// Exception thrown when trying to add a service object to an
/// execution_context where the service has a different owner.
class invalid_service_owner
  : public std::logic_error
{
public:
  ASIO_DECL invalid_service_owner();
};

namespace detail {

// Special derived service id type to keep classes header-file only.
template <typename Type>
class service_id
  : public execution_context::id
{
};

// Special service base class to keep classes header-file only.
template <typename Type>
class execution_context_service_base
  : public execution_context::service
{
public:
  static service_id<Type> id;

  // Constructor.
  execution_context_service_base(execution_context& e)
    : execution_context::service(e)
  {
  }
};

template <typename Type>
service_id<Type> execution_context_service_base<Type>::id;

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#include "asio/core/impl/execution_context.hpp"
#if defined(ASIO_HEADER_ONLY)
# include "asio/core/impl/execution_context.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_EXECUTION_CONTEXT_HPP
