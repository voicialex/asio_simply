#ifndef ASIO_IMPL_EXECUTOR_HPP
#define ASIO_IMPL_EXECUTOR_HPP

#include "asio/detail/config.hpp"
#include "asio/detail/base/stdcpp/atomic_count.hpp"
#include "asio/core/executor/executor_function.hpp"
#include "asio/detail/base/global.hpp"
#include "asio/detail/memory/memory.hpp"
#include "asio/detail/memory/recycling_allocator.hpp"
#include "asio/core/executor/executor.hpp"
#include "asio/core/executor/system_executor.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

// Lightweight, move-only function object wrapper.
class executor::function
{
public:
  template <typename F, typename Alloc>
  explicit function(F f, const Alloc& a)
  {
    // Allocate and construct an operation to wrap the function.
    typedef detail::executor_function<F, Alloc> func_type;
    typename func_type::ptr p = {
      detail::addressof(a), func_type::ptr::allocate(a), 0 };
    func_ = new (p.v) func_type(static_cast<F&&>(f), a);
    p.v = 0;
  }

  function(function&& other) ASIO_NOEXCEPT
    : func_(other.func_)
  {
    other.func_ = 0;
  }

  ~function()
  {
    if (func_)
      func_->destroy();
  }

  void operator()()
  {
    if (func_)
    {
      detail::executor_function_base* func = func_;
      func_ = 0;
      func->complete();
    }
  }

private:
  detail::executor_function_base* func_;
};

// Default polymorphic allocator implementation.
template <typename Executor, typename Allocator>
class executor::impl
  : public executor::impl_base
{
public:
  typedef ASIO_REBIND_ALLOC(Allocator, impl) allocator_type;

  static impl_base* create(const Executor& e, Allocator a = Allocator())
  {
    raw_mem mem(a);
    impl* p = new (mem.ptr_) impl(e, a);
    mem.ptr_ = 0;
    return p;
  }

  impl(const Executor& e, const Allocator& a) ASIO_NOEXCEPT
    : impl_base(false),
      ref_count_(1),
      executor_(e),
      allocator_(a)
  {
  }

  impl_base* clone() const ASIO_NOEXCEPT
  {
    ++ref_count_;
    return const_cast<impl_base*>(static_cast<const impl_base*>(this));
  }

  void destroy() ASIO_NOEXCEPT
  {
    if (--ref_count_ == 0)
    {
      allocator_type alloc(allocator_);
      impl* p = this;
      p->~impl();
      alloc.deallocate(p, 1);
    }
  }

  void on_work_started() ASIO_NOEXCEPT
  {
    executor_.on_work_started();
  }

  void on_work_finished() ASIO_NOEXCEPT
  {
    executor_.on_work_finished();
  }

  execution_context& context() ASIO_NOEXCEPT
  {
    return executor_.context();
  }

  void dispatch(function&& f)
  {
    executor_.dispatch(static_cast<function&&>(f), allocator_);
  }

  void post(function&& f)
  {
    executor_.post(static_cast<function&&>(f), allocator_);
  }

  void defer(function&& f)
  {
    executor_.defer(static_cast<function&&>(f), allocator_);
  }

  type_id_result_type target_type() const ASIO_NOEXCEPT
  {
    return type_id<Executor>();
  }

  void* target() ASIO_NOEXCEPT
  {
    return &executor_;
  }

  const void* target() const ASIO_NOEXCEPT
  {
    return &executor_;
  }

  bool equals(const impl_base* e) const ASIO_NOEXCEPT
  {
    if (this == e)
      return true;
    if (target_type() != e->target_type())
      return false;
    return executor_ == *static_cast<const Executor*>(e->target());
  }

private:
  mutable detail::atomic_count ref_count_;
  Executor executor_;
  Allocator allocator_;

  struct raw_mem
  {
    allocator_type allocator_;
    impl* ptr_;

    explicit raw_mem(const Allocator& a)
      : allocator_(a),
        ptr_(allocator_.allocate(1))
    {
    }

    ~raw_mem()
    {
      if (ptr_)
        allocator_.deallocate(ptr_, 1);
    }

  private:
    // Disallow copying and assignment.
    raw_mem(const raw_mem&);
    raw_mem operator=(const raw_mem&);
  };
};

// Polymorphic allocator specialisation for system_executor.
template <typename Allocator>
class executor::impl<system_executor, Allocator>
  : public executor::impl_base
{
public:
  static impl_base* create(const system_executor&,
      const Allocator& = Allocator())
  {
    return &detail::global<impl<system_executor, std::allocator<void> > >();
  }

  impl()
    : impl_base(true)
  {
  }

  impl_base* clone() const ASIO_NOEXCEPT
  {
    return const_cast<impl_base*>(static_cast<const impl_base*>(this));
  }

  void destroy() ASIO_NOEXCEPT
  {
  }

  void on_work_started() ASIO_NOEXCEPT
  {
    executor_.on_work_started();
  }

  void on_work_finished() ASIO_NOEXCEPT
  {
    executor_.on_work_finished();
  }

  execution_context& context() ASIO_NOEXCEPT
  {
    return executor_.context();
  }

  void dispatch(function&& f)
  {
    executor_.dispatch(static_cast<function&&>(f), allocator_);
  }

  void post(function&& f)
  {
    executor_.post(static_cast<function&&>(f), allocator_);
  }

  void defer(function&& f)
  {
    executor_.defer(static_cast<function&&>(f), allocator_);
  }

  type_id_result_type target_type() const ASIO_NOEXCEPT
  {
    return type_id<system_executor>();
  }

  void* target() ASIO_NOEXCEPT
  {
    return &executor_;
  }

  const void* target() const ASIO_NOEXCEPT
  {
    return &executor_;
  }

  bool equals(const impl_base* e) const ASIO_NOEXCEPT
  {
    return this == e;
  }

private:
  system_executor executor_;
  Allocator allocator_;
};

template <typename Executor>
executor::executor(Executor e)
  : impl_(impl<Executor, std::allocator<void> >::create(e))
{
}

template <typename Executor, typename Allocator>
executor::executor(allocator_arg_t, const Allocator& a, Executor e)
  : impl_(impl<Executor, Allocator>::create(e, a))
{
}

template <typename Function, typename Allocator>
void executor::dispatch(Function&& f,
    const Allocator& a) const
{
  impl_base* i = get_impl();
  if (i->fast_dispatch_)
    system_executor().dispatch(static_cast<Function&&>(f), a);
  else
    i->dispatch(function(static_cast<Function&&>(f), a));
}

template <typename Function, typename Allocator>
void executor::post(Function&& f,
    const Allocator& a) const
{
  get_impl()->post(function(static_cast<Function&&>(f), a));
}

template <typename Function, typename Allocator>
void executor::defer(Function&& f,
    const Allocator& a) const
{
  get_impl()->defer(function(static_cast<Function&&>(f), a));
}

template <typename Executor>
Executor* executor::target() ASIO_NOEXCEPT
{
  return impl_ && impl_->target_type() == type_id<Executor>()
    ? static_cast<Executor*>(impl_->target()) : 0;
}

template <typename Executor>
const Executor* executor::target() const ASIO_NOEXCEPT
{
  return impl_ && impl_->target_type() == type_id<Executor>()
    ? static_cast<Executor*>(impl_->target()) : 0;
}

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IMPL_EXECUTOR_HPP
