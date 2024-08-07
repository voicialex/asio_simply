#ifndef ASIO_IMPL_CONNECT_HPP
#define ASIO_IMPL_CONNECT_HPP

#include <algorithm>
#include "asio/detail/memory/associated_allocator.hpp"
#include "asio/core/executor/helper/associated_executor.hpp"
#include "asio/core/handler/bind_handler.hpp"
#include "asio/detail/memory/handler_alloc_helpers.hpp"
#include "asio/core/handler/handler_cont_helpers.hpp"
#include "asio/core/handler/handler_invoke_helpers.hpp"
#include "asio/core/handler/handler_type_requirements.hpp"
#include "asio/error/throw_error.hpp"
#include "asio/error/error.hpp"
#include "asio/core/executor/submit/post.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

namespace detail
{
  struct default_connect_condition
  {
    template <typename Endpoint>
    bool operator()(const asio::error_code&, const Endpoint&)
    {
      return true;
    }
  };

  template <typename Protocol, typename Iterator>
  inline typename Protocol::endpoint deref_connect_result(
      Iterator iter, asio::error_code& ec)
  {
    return ec ? typename Protocol::endpoint() : *iter;
  }

  template <typename T, typename Iterator>
  struct legacy_connect_condition_helper : T
  {
    typedef char (*fallback_func_type)(...);
    operator fallback_func_type() const;
  };

  template <typename R, typename Arg1, typename Arg2, typename Iterator>
  struct legacy_connect_condition_helper<R (*)(Arg1, Arg2), Iterator>
  {
    R operator()(Arg1, Arg2) const;
    char operator()(...) const;
  };

  template <typename T, typename Iterator>
  struct is_legacy_connect_condition
  {
    static char asio_connect_condition_check(char);
    static char (&asio_connect_condition_check(Iterator))[2];

    static const bool value =
      sizeof(asio_connect_condition_check(
        (*static_cast<legacy_connect_condition_helper<T, Iterator>*>(0))(
          *static_cast<const asio::error_code*>(0),
          *static_cast<const Iterator*>(0)))) != 1;
  };

  template <typename ConnectCondition, typename Iterator>
  inline Iterator call_connect_condition(ConnectCondition& connect_condition,
      const asio::error_code& ec, Iterator next, Iterator end,
      typename enable_if<is_legacy_connect_condition<
        ConnectCondition, Iterator>::value>::type* = 0)
  {
    if (next != end)
      return connect_condition(ec, next);
    return end;
  }

  template <typename ConnectCondition, typename Iterator>
  inline Iterator call_connect_condition(ConnectCondition& connect_condition,
      const asio::error_code& ec, Iterator next, Iterator end,
      typename enable_if<!is_legacy_connect_condition<
        ConnectCondition, Iterator>::value>::type* = 0)
  {
    for (;next != end; ++next)
      if (connect_condition(ec, *next))
        return next;
    return end;
  }
}

template <typename Protocol, typename EndpointSequence>
typename Protocol::endpoint connect(
    basic_socket<Protocol>& s,
    const EndpointSequence& endpoints,
    typename enable_if<is_endpoint_sequence<
        EndpointSequence>::value>::type*)
{
  asio::error_code ec;
  typename Protocol::endpoint result = connect(s, endpoints, ec);
  asio::detail::throw_error(ec, "connect");
  return result;
}

template <typename Protocol, typename EndpointSequence>
typename Protocol::endpoint connect(
    basic_socket<Protocol>& s,
    const EndpointSequence& endpoints, asio::error_code& ec,
    typename enable_if<is_endpoint_sequence<
        EndpointSequence>::value>::type*)
{
  return detail::deref_connect_result<Protocol>(
      connect(s, endpoints.begin(), endpoints.end(),
        detail::default_connect_condition(), ec), ec);
}

template <typename Protocol, typename Iterator>
Iterator connect(basic_socket<Protocol>& s,
    Iterator begin, Iterator end)
{
  asio::error_code ec;
  Iterator result = connect(s, begin, end, ec);
  asio::detail::throw_error(ec, "connect");
  return result;
}

template <typename Protocol, typename Iterator>
inline Iterator connect(basic_socket<Protocol>& s,
    Iterator begin, Iterator end, asio::error_code& ec)
{
  return connect(s, begin, end, detail::default_connect_condition(), ec);
}

template <typename Protocol,
    typename EndpointSequence, typename ConnectCondition>
typename Protocol::endpoint connect(
    basic_socket<Protocol>& s,
    const EndpointSequence& endpoints, ConnectCondition connect_condition,
    typename enable_if<is_endpoint_sequence<
        EndpointSequence>::value>::type*)
{
  asio::error_code ec;
  typename Protocol::endpoint result = connect(
      s, endpoints, connect_condition, ec);
  asio::detail::throw_error(ec, "connect");
  return result;
}

template <typename Protocol,
    typename EndpointSequence, typename ConnectCondition>
typename Protocol::endpoint connect(
    basic_socket<Protocol>& s,
    const EndpointSequence& endpoints, ConnectCondition connect_condition,
    asio::error_code& ec,
    typename enable_if<is_endpoint_sequence<
        EndpointSequence>::value>::type*)
{
  return detail::deref_connect_result<Protocol>(
      connect(s, endpoints.begin(), endpoints.end(),
        connect_condition, ec), ec);
}

template <typename Protocol,
    typename Iterator, typename ConnectCondition>
Iterator connect(basic_socket<Protocol>& s,
    Iterator begin, Iterator end, ConnectCondition connect_condition)
{
  asio::error_code ec;
  Iterator result = connect(s, begin, end, connect_condition, ec);
  asio::detail::throw_error(ec, "connect");
  return result;
}

template <typename Protocol,
    typename Iterator, typename ConnectCondition>
Iterator connect(basic_socket<Protocol>& s,
    Iterator begin, Iterator end, ConnectCondition connect_condition,
    asio::error_code& ec)
{
  ec = asio::error_code();

  for (Iterator iter = begin; iter != end; ++iter)
  {
    iter = (detail::call_connect_condition(connect_condition, ec, iter, end));
    if (iter != end)
    {
      s.close(ec);
      s.connect(*iter, ec);
      if (!ec)
        return iter;
    }
    else
      break;
  }

  if (!ec)
    ec = asio::error::not_found;

  return end;
}

namespace detail
{
  // Enable the empty base class optimisation for the connect condition.
  template <typename ConnectCondition>
  class base_from_connect_condition
  {
  protected:
    explicit base_from_connect_condition(
        const ConnectCondition& connect_condition)
      : connect_condition_(connect_condition)
    {
    }

    template <typename Iterator>
    void check_condition(const asio::error_code& ec,
        Iterator& iter, Iterator& end)
    {
      iter = detail::call_connect_condition(connect_condition_, ec, iter, end);
    }

  private:
    ConnectCondition connect_condition_;
  };

  // The default_connect_condition implementation is essentially a no-op. This
  // template specialisation lets us eliminate all costs associated with it.
  template <>
  class base_from_connect_condition<default_connect_condition>
  {
  protected:
    explicit base_from_connect_condition(const default_connect_condition&)
    {
    }

    template <typename Iterator>
    void check_condition(const asio::error_code&, Iterator&, Iterator&)
    {
    }
  };

  template <typename Protocol,
      typename EndpointSequence, typename ConnectCondition,
      typename RangeConnectHandler>
  class range_connect_op : base_from_connect_condition<ConnectCondition>
  {
  public:
    range_connect_op(basic_socket<Protocol>& sock,
        const EndpointSequence& endpoints,
        const ConnectCondition& connect_condition,
        RangeConnectHandler& handler)
      : base_from_connect_condition<ConnectCondition>(connect_condition),
        socket_(sock),
        endpoints_(endpoints),
        index_(0),
        start_(0),
        handler_(static_cast<RangeConnectHandler&&>(handler))
    {
    }

#if defined(ASIO_HAS_MOVE)
    range_connect_op(const range_connect_op& other)
      : base_from_connect_condition<ConnectCondition>(other),
        socket_(other.socket_),
        endpoints_(other.endpoints_),
        index_(other.index_),
        start_(other.start_),
        handler_(other.handler_)
    {
    }

    range_connect_op(range_connect_op&& other)
      : base_from_connect_condition<ConnectCondition>(other),
        socket_(other.socket_),
        endpoints_(other.endpoints_),
        index_(other.index_),
        start_(other.start_),
        handler_(static_cast<RangeConnectHandler&&>(other.handler_))
    {
    }
#endif // defined(ASIO_HAS_MOVE)

    void operator()(asio::error_code ec, int start = 0)
    {
      typename EndpointSequence::const_iterator begin = endpoints_.begin();
      typename EndpointSequence::const_iterator iter = begin;
      std::advance(iter, index_);
      typename EndpointSequence::const_iterator end = endpoints_.end();

      switch (start_ = start)
      {
        case 1:
        for (;;)
        {
          this->check_condition(ec, iter, end);
          index_ = std::distance(begin, iter);

          if (iter != end)
          {
            socket_.close(ec);
            socket_.async_connect(*iter,
                static_cast<range_connect_op&&>(*this));
            return;
          }

          if (start)
          {
            ec = asio::error::not_found;
            asio::post(socket_.get_executor(),
                detail::bind_handler(
                  static_cast<range_connect_op&&>(*this), ec));
            return;
          }

          default:

          if (iter == end)
            break;

          if (!socket_.is_open())
          {
            ec = asio::error::operation_aborted;
            break;
          }

          if (!ec)
            break;

          ++iter;
          ++index_;
        }

        handler_(static_cast<const asio::error_code&>(ec),
            static_cast<const typename Protocol::endpoint&>(
              ec || iter == end ? typename Protocol::endpoint() : *iter));
      }
    }

  //private:
    basic_socket<Protocol>& socket_;
    EndpointSequence endpoints_;
    std::size_t index_;
    int start_;
    RangeConnectHandler handler_;
  };

  template <typename Protocol,
      typename EndpointSequence, typename ConnectCondition,
      typename RangeConnectHandler>
  inline void* asio_handler_allocate(std::size_t size,
      range_connect_op<Protocol, EndpointSequence,
        ConnectCondition, RangeConnectHandler>* this_handler)
  {
    return asio_handler_alloc_helpers::allocate(
        size, this_handler->handler_);
  }

  template <typename Protocol,
      typename EndpointSequence, typename ConnectCondition,
      typename RangeConnectHandler>
  inline void asio_handler_deallocate(void* pointer, std::size_t size,
      range_connect_op<Protocol, EndpointSequence,
        ConnectCondition, RangeConnectHandler>* this_handler)
  {
    asio_handler_alloc_helpers::deallocate(
        pointer, size, this_handler->handler_);
  }

  template <typename Protocol,
      typename EndpointSequence, typename ConnectCondition,
      typename RangeConnectHandler>
  inline bool asio_handler_is_continuation(
      range_connect_op<Protocol, EndpointSequence,
        ConnectCondition, RangeConnectHandler>* this_handler)
  {
    return asio_handler_cont_helpers::is_continuation(
        this_handler->handler_);
  }

  template <typename Function, typename Protocol, typename EndpointSequence,
      typename ConnectCondition, typename RangeConnectHandler>
  inline void asio_handler_invoke(Function& function,
      range_connect_op<Protocol, EndpointSequence,
        ConnectCondition, RangeConnectHandler>* this_handler)
  {
    asio_handler_invoke_helpers::invoke(
        function, this_handler->handler_);
  }

  template <typename Function, typename Protocol, typename EndpointSequence,
      typename ConnectCondition, typename RangeConnectHandler>
  inline void asio_handler_invoke(const Function& function,
      range_connect_op<Protocol, EndpointSequence,
        ConnectCondition, RangeConnectHandler>* this_handler)
  {
    asio_handler_invoke_helpers::invoke(
        function, this_handler->handler_);
  }

  template <typename Protocol, typename Iterator,
      typename ConnectCondition, typename IteratorConnectHandler>
  class iterator_connect_op : base_from_connect_condition<ConnectCondition>
  {
  public:
    iterator_connect_op(basic_socket<Protocol>& sock,
        const Iterator& begin, const Iterator& end,
        const ConnectCondition& connect_condition,
        IteratorConnectHandler& handler)
      : base_from_connect_condition<ConnectCondition>(connect_condition),
        socket_(sock),
        iter_(begin),
        end_(end),
        start_(0),
        handler_(static_cast<IteratorConnectHandler&&>(handler))
    {
    }

#if defined(ASIO_HAS_MOVE)
    iterator_connect_op(const iterator_connect_op& other)
      : base_from_connect_condition<ConnectCondition>(other),
        socket_(other.socket_),
        iter_(other.iter_),
        end_(other.end_),
        start_(other.start_),
        handler_(other.handler_)
    {
    }

    iterator_connect_op(iterator_connect_op&& other)
      : base_from_connect_condition<ConnectCondition>(other),
        socket_(other.socket_),
        iter_(other.iter_),
        end_(other.end_),
        start_(other.start_),
        handler_(static_cast<IteratorConnectHandler&&>(other.handler_))
    {
    }
#endif // defined(ASIO_HAS_MOVE)

    void operator()(asio::error_code ec, int start = 0)
    {
      switch (start_ = start)
      {
        case 1:
        for (;;)
        {
          this->check_condition(ec, iter_, end_);

          if (iter_ != end_)
          {
            socket_.close(ec);
            socket_.async_connect(*iter_,
                static_cast<iterator_connect_op&&>(*this));
            return;
          }

          if (start)
          {
            ec = asio::error::not_found;
            asio::post(socket_.get_executor(),
                detail::bind_handler(
                  static_cast<iterator_connect_op&&>(*this), ec));
            return;
          }

          default:

          if (iter_ == end_)
            break;

          if (!socket_.is_open())
          {
            ec = asio::error::operation_aborted;
            break;
          }

          if (!ec)
            break;

          ++iter_;
        }

        handler_(static_cast<const asio::error_code&>(ec),
            static_cast<const Iterator&>(iter_));
      }
    }

  //private:
    basic_socket<Protocol>& socket_;
    Iterator iter_;
    Iterator end_;
    int start_;
    IteratorConnectHandler handler_;
  };

  template <typename Protocol, typename Iterator,
      typename ConnectCondition, typename IteratorConnectHandler>
  inline void* asio_handler_allocate(std::size_t size,
      iterator_connect_op<Protocol, Iterator,
        ConnectCondition, IteratorConnectHandler>* this_handler)
  {
    return asio_handler_alloc_helpers::allocate(
        size, this_handler->handler_);
  }

  template <typename Protocol, typename Iterator,
      typename ConnectCondition, typename IteratorConnectHandler>
  inline void asio_handler_deallocate(void* pointer, std::size_t size,
      iterator_connect_op<Protocol, Iterator,
        ConnectCondition, IteratorConnectHandler>* this_handler)
  {
    asio_handler_alloc_helpers::deallocate(
        pointer, size, this_handler->handler_);
  }

  template <typename Protocol, typename Iterator,
      typename ConnectCondition, typename IteratorConnectHandler>
  inline bool asio_handler_is_continuation(
      iterator_connect_op<Protocol, Iterator,
        ConnectCondition, IteratorConnectHandler>* this_handler)
  {
    return asio_handler_cont_helpers::is_continuation(
        this_handler->handler_);
  }

  template <typename Function, typename Protocol, typename Iterator,
      typename ConnectCondition, typename IteratorConnectHandler>
  inline void asio_handler_invoke(Function& function,
      iterator_connect_op<Protocol, Iterator,
        ConnectCondition, IteratorConnectHandler>* this_handler)
  {
    asio_handler_invoke_helpers::invoke(
        function, this_handler->handler_);
  }

  template <typename Function, typename Protocol, typename Iterator,
      typename ConnectCondition, typename IteratorConnectHandler>
  inline void asio_handler_invoke(const Function& function,
      iterator_connect_op<Protocol, Iterator,
        ConnectCondition, IteratorConnectHandler>* this_handler)
  {
    asio_handler_invoke_helpers::invoke(
        function, this_handler->handler_);
  }
} // namespace detail

#if !defined(GENERATING_DOCUMENTATION)

template <typename Protocol,
    typename EndpointSequence, typename ConnectCondition,
    typename RangeConnectHandler, typename Allocator>
struct associated_allocator<
    detail::range_connect_op<Protocol,
      EndpointSequence, ConnectCondition, RangeConnectHandler>,
    Allocator>
{
  typedef typename associated_allocator<
      RangeConnectHandler, Allocator>::type type;

  static type get(
      const detail::range_connect_op<Protocol,
        EndpointSequence, ConnectCondition, RangeConnectHandler>& h,
      const Allocator& a = Allocator()) ASIO_NOEXCEPT
  {
    return associated_allocator<RangeConnectHandler,
        Allocator>::get(h.handler_, a);
  }
};

template <typename Protocol,
    typename EndpointSequence, typename ConnectCondition,
    typename RangeConnectHandler, typename Executor>
struct associated_executor<
    detail::range_connect_op<Protocol,
      EndpointSequence, ConnectCondition, RangeConnectHandler>,
    Executor>
{
  typedef typename associated_executor<
      RangeConnectHandler, Executor>::type type;

  static type get(
      const detail::range_connect_op<Protocol,
        EndpointSequence, ConnectCondition, RangeConnectHandler>& h,
      const Executor& ex = Executor()) ASIO_NOEXCEPT
  {
    return associated_executor<RangeConnectHandler,
        Executor>::get(h.handler_, ex);
  }
};

template <typename Protocol,
    typename Iterator, typename ConnectCondition,
    typename IteratorConnectHandler, typename Allocator>
struct associated_allocator<
    detail::iterator_connect_op<Protocol, Iterator,
      ConnectCondition, IteratorConnectHandler>,
    Allocator>
{
  typedef typename associated_allocator<
      IteratorConnectHandler, Allocator>::type type;

  static type get(
      const detail::iterator_connect_op<Protocol,
        Iterator, ConnectCondition, IteratorConnectHandler>& h,
      const Allocator& a = Allocator()) ASIO_NOEXCEPT
  {
    return associated_allocator<IteratorConnectHandler,
        Allocator>::get(h.handler_, a);
  }
};

template <typename Protocol,
    typename Iterator, typename ConnectCondition,
    typename IteratorConnectHandler, typename Executor>
struct associated_executor<
    detail::iterator_connect_op<Protocol, Iterator,
      ConnectCondition, IteratorConnectHandler>,
    Executor>
{
  typedef typename associated_executor<
      IteratorConnectHandler, Executor>::type type;

  static type get(
      const detail::iterator_connect_op<Protocol,
        Iterator, ConnectCondition, IteratorConnectHandler>& h,
      const Executor& ex = Executor()) ASIO_NOEXCEPT
  {
    return associated_executor<IteratorConnectHandler,
        Executor>::get(h.handler_, ex);
  }
};

#endif // !defined(GENERATING_DOCUMENTATION)

template <typename Protocol,
    typename EndpointSequence, typename RangeConnectHandler>
inline ASIO_INITFN_RESULT_TYPE(RangeConnectHandler,
    void (asio::error_code, typename Protocol::endpoint))
async_connect(basic_socket<Protocol>& s,
    const EndpointSequence& endpoints,
    RangeConnectHandler&& handler,
    typename enable_if<is_endpoint_sequence<
        EndpointSequence>::value>::type*)
{
  // If you get an error on the following line it means that your handler does
  // not meet the documented type requirements for a RangeConnectHandler.
  ASIO_RANGE_CONNECT_HANDLER_CHECK(
      RangeConnectHandler, handler, typename Protocol::endpoint) type_check;

  async_completion<RangeConnectHandler,
    void (asio::error_code, typename Protocol::endpoint)>
      init(handler);

  detail::range_connect_op<Protocol, EndpointSequence,
    detail::default_connect_condition,
      ASIO_HANDLER_TYPE(RangeConnectHandler,
        void (asio::error_code, typename Protocol::endpoint))>(s,
          endpoints, detail::default_connect_condition(),
            init.completion_handler)(asio::error_code(), 1);

  return init.result.get();
}

template <typename Protocol,
    typename Iterator, typename IteratorConnectHandler>
inline ASIO_INITFN_RESULT_TYPE(IteratorConnectHandler,
    void (asio::error_code, Iterator))
async_connect(basic_socket<Protocol>& s,
    Iterator begin, Iterator end,
    IteratorConnectHandler&& handler)
{
  // If you get an error on the following line it means that your handler does
  // not meet the documented type requirements for a IteratorConnectHandler.
  ASIO_ITERATOR_CONNECT_HANDLER_CHECK(
      IteratorConnectHandler, handler, Iterator) type_check;

  async_completion<IteratorConnectHandler,
    void (asio::error_code, Iterator)> init(handler);

  detail::iterator_connect_op<Protocol, Iterator,
    detail::default_connect_condition, ASIO_HANDLER_TYPE(
      IteratorConnectHandler, void (asio::error_code, Iterator))>(s,
        begin, end, detail::default_connect_condition(),
          init.completion_handler)(asio::error_code(), 1);

  return init.result.get();
}

template <typename Protocol, typename EndpointSequence,
    typename ConnectCondition, typename RangeConnectHandler>
inline ASIO_INITFN_RESULT_TYPE(RangeConnectHandler,
    void (asio::error_code, typename Protocol::endpoint))
async_connect(basic_socket<Protocol>& s,
    const EndpointSequence& endpoints, ConnectCondition connect_condition,
    RangeConnectHandler&& handler,
    typename enable_if<is_endpoint_sequence<
        EndpointSequence>::value>::type*)
{
  // If you get an error on the following line it means that your handler does
  // not meet the documented type requirements for a RangeConnectHandler.
  ASIO_RANGE_CONNECT_HANDLER_CHECK(
      RangeConnectHandler, handler, typename Protocol::endpoint) type_check;

  async_completion<RangeConnectHandler,
    void (asio::error_code, typename Protocol::endpoint)>
      init(handler);

  detail::range_connect_op<Protocol, EndpointSequence,
    ConnectCondition, ASIO_HANDLER_TYPE(RangeConnectHandler,
      void (asio::error_code, typename Protocol::endpoint))>(s,
        endpoints, connect_condition, init.completion_handler)(
          asio::error_code(), 1);

  return init.result.get();
}

template <typename Protocol, typename Iterator,
    typename ConnectCondition, typename IteratorConnectHandler>
inline ASIO_INITFN_RESULT_TYPE(IteratorConnectHandler,
    void (asio::error_code, Iterator))
async_connect(basic_socket<Protocol>& s,
    Iterator begin, Iterator end, ConnectCondition connect_condition,
    IteratorConnectHandler&& handler)
{
  // If you get an error on the following line it means that your handler does
  // not meet the documented type requirements for a IteratorConnectHandler.
  ASIO_ITERATOR_CONNECT_HANDLER_CHECK(
      IteratorConnectHandler, handler, Iterator) type_check;

  async_completion<IteratorConnectHandler,
    void (asio::error_code, Iterator)> init(handler);

  detail::iterator_connect_op<Protocol, Iterator,
    ConnectCondition, ASIO_HANDLER_TYPE(
      IteratorConnectHandler, void (asio::error_code, Iterator))>(s,
        begin, end, connect_condition, init.completion_handler)(
          asio::error_code(), 1);

  return init.result.get();
}

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IMPL_CONNECT_HPP
