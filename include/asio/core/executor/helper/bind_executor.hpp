#ifndef ASIO_BIND_EXECUTOR_HPP
#define ASIO_BIND_EXECUTOR_HPP

#include "asio/detail/config.hpp"
#include "asio/detail/base/stdcpp/type_traits.hpp"
// #include "asio/detail/variadic_templates.hpp"
#include "asio/core/executor/helper/associated_executor.hpp"
#include "asio/detail/memory/associated_allocator.hpp"
#include "asio/core/executor/helper/async_result.hpp"
#include "asio/core/execution_context.hpp"
#include "asio/core/executor/is_executor.hpp"
#include "asio/core/executor/helper/uses_executor.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename T>
struct executor_binder_check
{
  typedef void type;
};

// Helper to automatically define nested typedef result_type.

template <typename T, typename = void>
struct executor_binder_result_type
{
protected:
  typedef void result_type_or_void;
};

template <typename T>
struct executor_binder_result_type<T,
  typename executor_binder_check<typename T::result_type>::type>
{
  typedef typename T::result_type result_type;
protected:
  typedef result_type result_type_or_void;
};

template <typename R>
struct executor_binder_result_type<R(*)()>
{
  typedef R result_type;
protected:
  typedef result_type result_type_or_void;
};

template <typename R>
struct executor_binder_result_type<R(&)()>
{
  typedef R result_type;
protected:
  typedef result_type result_type_or_void;
};

template <typename R, typename A1>
struct executor_binder_result_type<R(*)(A1)>
{
  typedef R result_type;
protected:
  typedef result_type result_type_or_void;
};

template <typename R, typename A1>
struct executor_binder_result_type<R(&)(A1)>
{
  typedef R result_type;
protected:
  typedef result_type result_type_or_void;
};

template <typename R, typename A1, typename A2>
struct executor_binder_result_type<R(*)(A1, A2)>
{
  typedef R result_type;
protected:
  typedef result_type result_type_or_void;
};

template <typename R, typename A1, typename A2>
struct executor_binder_result_type<R(&)(A1, A2)>
{
  typedef R result_type;
protected:
  typedef result_type result_type_or_void;
};

// Helper to automatically define nested typedef argument_type.

template <typename T, typename = void>
struct executor_binder_argument_type {};

template <typename T>
struct executor_binder_argument_type<T,
  typename executor_binder_check<typename T::argument_type>::type>
{
  typedef typename T::argument_type argument_type;
};

template <typename R, typename A1>
struct executor_binder_argument_type<R(*)(A1)>
{
  typedef A1 argument_type;
};

template <typename R, typename A1>
struct executor_binder_argument_type<R(&)(A1)>
{
  typedef A1 argument_type;
};

// Helper to automatically define nested typedefs first_argument_type and
// second_argument_type.

template <typename T, typename = void>
struct executor_binder_argument_types {};

template <typename T>
struct executor_binder_argument_types<T,
  typename executor_binder_check<typename T::first_argument_type>::type>
{
  typedef typename T::first_argument_type first_argument_type;
  typedef typename T::second_argument_type second_argument_type;
};

template <typename R, typename A1, typename A2>
struct executor_binder_argument_type<R(*)(A1, A2)>
{
  typedef A1 first_argument_type;
  typedef A2 second_argument_type;
};

template <typename R, typename A1, typename A2>
struct executor_binder_argument_type<R(&)(A1, A2)>
{
  typedef A1 first_argument_type;
  typedef A2 second_argument_type;
};

// Helper to:
// - Apply the empty base optimisation to the executor.
// - Perform uses_executor construction of the target type, if required.

template <typename T, typename Executor, bool UsesExecutor>
class executor_binder_base;

template <typename T, typename Executor>
class executor_binder_base<T, Executor, true>
  : protected Executor
{
protected:
  template <typename E, typename U>
  executor_binder_base(E&& e, U&& u)
    : executor_(static_cast<E&&>(e)),
      target_(executor_arg_t(), executor_, static_cast<U&&>(u))
  {
  }

  Executor executor_;
  T target_;
};

template <typename T, typename Executor>
class executor_binder_base<T, Executor, false>
{
protected:
  template <typename E, typename U>
  executor_binder_base(E&& e, U&& u)
    : executor_(static_cast<E&&>(e)),
      target_(static_cast<U&&>(u))
  {
  }

  Executor executor_;
  T target_;
};

// Helper to enable SFINAE on zero-argument operator() below.

template <typename T, typename = void>
struct executor_binder_result_of0
{
  typedef void type;
};

template <typename T>
struct executor_binder_result_of0<T,
  typename executor_binder_check<typename result_of<T()>::type>::type>
{
  typedef typename result_of<T()>::type type;
};

} // namespace detail

/// A call wrapper type to bind an executor of type @c Executor to an object of
/// type @c T.
template <typename T, typename Executor>
class executor_binder
  : public detail::executor_binder_result_type<T>,
    public detail::executor_binder_argument_type<T>,
    public detail::executor_binder_argument_types<T>,
    private detail::executor_binder_base<
      T, Executor, uses_executor<T, Executor>::value>
{
public:
  /// The type of the target object.
  typedef T target_type;

  /// The type of the associated executor.
  typedef Executor executor_type;

  /// Construct an executor wrapper for the specified object.
  /**
   * This constructor is only valid if the type @c T is constructible from type
   * @c U.
   */
  template <typename U>
  executor_binder(executor_arg_t, const executor_type& e,
      U&& u)
    : base_type(e, static_cast<U&&>(u))
  {
  }

  /// Copy constructor.
  executor_binder(const executor_binder& other)
    : base_type(other.get_executor(), other.get())
  {
  }

  /// Construct a copy, but specify a different executor.
  executor_binder(executor_arg_t, const executor_type& e,
      const executor_binder& other)
    : base_type(e, other.get())
  {
  }

  /// Construct a copy of a different executor wrapper type.
  /**
   * This constructor is only valid if the @c Executor type is constructible
   * from type @c OtherExecutor, and the type @c T is constructible from type
   * @c U.
   */
  template <typename U, typename OtherExecutor>
  executor_binder(const executor_binder<U, OtherExecutor>& other)
    : base_type(other.get_executor(), other.get())
  {
  }

  /// Construct a copy of a different executor wrapper type, but specify a
  /// different executor.
  /**
   * This constructor is only valid if the type @c T is constructible from type
   * @c U.
   */
  template <typename U, typename OtherExecutor>
  executor_binder(executor_arg_t, const executor_type& e,
      const executor_binder<U, OtherExecutor>& other)
    : base_type(e, other.get())
  {
  }

  /// Move constructor.
  executor_binder(executor_binder&& other)
    : base_type(static_cast<executor_type&&>(other.get_executor()),
        static_cast<T&&>(other.get()))
  {
  }

  /// Move construct the target object, but specify a different executor.
  executor_binder(executor_arg_t, const executor_type& e,
      executor_binder&& other)
    : base_type(e, static_cast<T&&>(other.get()))
  {
  }

  /// Move construct from a different executor wrapper type.
  template <typename U, typename OtherExecutor>
  executor_binder(executor_binder<U, OtherExecutor>&& other)
    : base_type(static_cast<OtherExecutor&&>(other.get_executor()),
        static_cast<U&&>(other.get()))
  {
  }

  /// Move construct from a different executor wrapper type, but specify a
  /// different executor.
  template <typename U, typename OtherExecutor>
  executor_binder(executor_arg_t, const executor_type& e,
      executor_binder<U, OtherExecutor>&& other)
    : base_type(e, static_cast<U&&>(other.get()))
  {
  }

  /// Destructor.
  ~executor_binder()
  {
  }

  /// Obtain a reference to the target object.
  target_type& get() ASIO_NOEXCEPT
  {
    return this->target_;
  }

  /// Obtain a reference to the target object.
  const target_type& get() const ASIO_NOEXCEPT
  {
    return this->target_;
  }

  /// Obtain the associated executor.
  executor_type get_executor() const ASIO_NOEXCEPT
  {
    return this->executor_;
  }

#if defined(ASIO_HAS_VARIADIC_TEMPLATES)

  /// Forwarding function call operator.
  template <typename... Args>
  typename result_of<T(Args...)>::type operator()(
      Args&&... args)
  {
    return this->target_(static_cast<Args&&>(args)...);
  }

  /// Forwarding function call operator.
  template <typename... Args>
  typename result_of<T(Args...)>::type operator()(
      Args&&... args) const
  {
    return this->target_(static_cast<Args&&>(args)...);
  }

#elif defined(ASIO_HAS_STD_TYPE_TRAITS) && !defined(_MSC_VER)

  typename detail::executor_binder_result_of0<T>::type operator()()
  {
    return this->target_();
  }

  typename detail::executor_binder_result_of0<T>::type operator()() const
  {
    return this->target_();
  }

#define ASIO_PRIVATE_BIND_EXECUTOR_CALL_DEF(n) \
  template <ASIO_VARIADIC_TPARAMS(n)> \
  typename result_of<T(ASIO_VARIADIC_TARGS(n))>::type operator()( \
      ASIO_VARIADIC_MOVE_PARAMS(n)) \
  { \
    return this->target_(ASIO_VARIADIC_MOVE_ARGS(n)); \
  } \
  \
  template <ASIO_VARIADIC_TPARAMS(n)> \
  typename result_of<T(ASIO_VARIADIC_TARGS(n))>::type operator()( \
      ASIO_VARIADIC_MOVE_PARAMS(n)) const \
  { \
    return this->target_(ASIO_VARIADIC_MOVE_ARGS(n)); \
  } \
  /**/
  ASIO_VARIADIC_GENERATE(ASIO_PRIVATE_BIND_EXECUTOR_CALL_DEF)
#undef ASIO_PRIVATE_BIND_EXECUTOR_CALL_DEF

#else // defined(ASIO_HAS_STD_TYPE_TRAITS) && !defined(_MSC_VER)

  typedef typename detail::executor_binder_result_type<T>::result_type_or_void
    result_type_or_void;

  result_type_or_void operator()()
  {
    return this->target_();
  }

  result_type_or_void operator()() const
  {
    return this->target_();
  }

#define ASIO_PRIVATE_BIND_EXECUTOR_CALL_DEF(n) \
  template <ASIO_VARIADIC_TPARAMS(n)> \
  result_type_or_void operator()( \
      ASIO_VARIADIC_MOVE_PARAMS(n)) \
  { \
    return this->target_(ASIO_VARIADIC_MOVE_ARGS(n)); \
  } \
  \
  template <ASIO_VARIADIC_TPARAMS(n)> \
  result_type_or_void operator()( \
      ASIO_VARIADIC_MOVE_PARAMS(n)) const \
  { \
    return this->target_(ASIO_VARIADIC_MOVE_ARGS(n)); \
  } \
  /**/
  ASIO_VARIADIC_GENERATE(ASIO_PRIVATE_BIND_EXECUTOR_CALL_DEF)
#undef ASIO_PRIVATE_BIND_EXECUTOR_CALL_DEF

#endif // defined(ASIO_HAS_STD_TYPE_TRAITS) && !defined(_MSC_VER)

private:
  typedef detail::executor_binder_base<T, Executor,
    uses_executor<T, Executor>::value> base_type;
};

/// Associate an object of type @c T with an executor of type @c Executor.
template <typename Executor, typename T>
inline executor_binder<typename decay<T>::type, Executor>
bind_executor(const Executor& ex, T&& t,
    typename enable_if<is_executor<Executor>::value>::type* = 0)
{
  return executor_binder<typename decay<T>::type, Executor>(
      executor_arg_t(), ex, static_cast<T&&>(t));
}

/// Associate an object of type @c T with an execution context's executor.
template <typename ExecutionContext, typename T>
inline executor_binder<typename decay<T>::type,
  typename ExecutionContext::executor_type>
bind_executor(ExecutionContext& ctx, T&& t,
    typename enable_if<is_convertible<
      ExecutionContext&, execution_context&>::value>::type* = 0)
{
  return executor_binder<typename decay<T>::type,
    typename ExecutionContext::executor_type>(
      executor_arg_t(), ctx.get_executor(), static_cast<T&&>(t));
}

#if !defined(GENERATING_DOCUMENTATION)

template <typename T, typename Executor>
struct uses_executor<executor_binder<T, Executor>, Executor>
  : true_type {};

template <typename T, typename Executor, typename Signature>
class async_result<executor_binder<T, Executor>, Signature>
{
public:
  typedef executor_binder<
    typename async_result<T, Signature>::completion_handler_type, Executor>
      completion_handler_type;

  typedef typename async_result<T, Signature>::return_type return_type;

  explicit async_result(executor_binder<T, Executor>& b)
    : target_(b.get())
  {
  }

  return_type get()
  {
    return target_.get();
  }

private:
  async_result(const async_result&) ASIO_DELETED;
  async_result& operator=(const async_result&) ASIO_DELETED;

  async_result<T, Signature> target_;
};

template <typename T, typename Executor, typename Allocator>
struct associated_allocator<executor_binder<T, Executor>, Allocator>
{
  typedef typename associated_allocator<T, Allocator>::type type;

  static type get(const executor_binder<T, Executor>& b,
      const Allocator& a = Allocator()) ASIO_NOEXCEPT
  {
    return associated_allocator<T, Allocator>::get(b.get(), a);
  }
};

template <typename T, typename Executor, typename Executor1>
struct associated_executor<executor_binder<T, Executor>, Executor1>
{
  typedef Executor type;

  static type get(const executor_binder<T, Executor>& b,
      const Executor1& = Executor1()) ASIO_NOEXCEPT
  {
    return b.get_executor();
  }
};

#endif // !defined(GENERATING_DOCUMENTATION)

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_BIND_EXECUTOR_HPP