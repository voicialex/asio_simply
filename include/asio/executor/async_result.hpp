#ifndef ASIO_ASYNC_RESULT_HPP
#define ASIO_ASYNC_RESULT_HPP

#include "asio/detail/config.hpp"
#include "asio/detail/base/impl_std/type_traits.hpp"
#include "asio/handler_type.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

/// An interface for customising the behaviour of an initiating function.
/**
 * The async_result traits class is used for determining:
 *
 * @li the concrete completion handler type to be called at the end of the
 * asynchronous operation;
 *
 * @li the initiating function return type; and
 *
 * @li how the return value of the initiating function is obtained.
 *
 * The trait allows the handler and return types to be determined at the point
 * where the specific completion handler signature is known.
 *
 * This template may be specialised for user-defined completion token types.
 * The primary template assumes that the CompletionToken is the completion
 * handler.
 */

template <typename CompletionToken, typename Signature = void>

class async_result
{
public:
  // For backward compatibility, determine the concrete completion handler type
  // by using the legacy handler_type trait.
  typedef typename handler_type<CompletionToken, Signature>::type
    completion_handler_type;

  // For backward compatibility, determine the initiating function return type
  // using the legacy single-parameter version of async_result.
  typedef typename async_result<completion_handler_type>::type return_type;

  /// Construct an async result from a given handler.
  /**
   * When using a specalised async_result, the constructor has an opportunity
   * to initialise some state associated with the completion handler, which is
   * then returned from the initiating function.
   */
  explicit async_result(completion_handler_type& h)
    : legacy_result_(h)
  {
    (void)h;
  }

  /// Obtain the value to be returned from the initiating function.
  return_type get()
  {
    return legacy_result_.get();
  }

private:
  async_result(const async_result&) ASIO_DELETED;
  async_result& operator=(const async_result&) ASIO_DELETED;

  async_result<completion_handler_type> legacy_result_;
};

/// (Deprecated: Use two-parameter version of async_result.) An interface for
/// customising the behaviour of an initiating function.
/**
 * This template may be specialised for user-defined handler types.
 */
template <typename Handler>
class async_result<Handler>
{
public:
  /// The return type of the initiating function.
  typedef void type;

  /// Construct an async result from a given handler.
  /**
   * When using a specalised async_result, the constructor has an opportunity
   * to initialise some state associated with the handler, which is then
   * returned from the initiating function.
   */
  explicit async_result(Handler&)
  {
  }

  /// Obtain the value to be returned from the initiating function.
  type get()
  {
  }
};

/// Helper template to deduce the handler type from a CompletionToken, capture
/// a local copy of the handler, and then create an async_result for the
/// handler.
template <typename CompletionToken, typename Signature>
struct async_completion
{
  /// The real handler type to be used for the asynchronous operation.
  typedef typename asio::async_result<
    typename decay<CompletionToken>::type,
      Signature>::completion_handler_type completion_handler_type;

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Constructor.
  /**
   * The constructor creates the concrete completion handler and makes the link
   * between the handler and the asynchronous result.
   */
  explicit async_completion(CompletionToken& token)
    : completion_handler(static_cast<typename conditional<
        is_same<CompletionToken, completion_handler_type>::value,
        completion_handler_type&, CompletionToken&&>::type>(token)),
      result(completion_handler)
  {
  }
#else // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  explicit async_completion(typename decay<CompletionToken>::type& token)
    : completion_handler(token),
      result(completion_handler)
  {
  }

  explicit async_completion(const typename decay<CompletionToken>::type& token)
    : completion_handler(token),
      result(completion_handler)
  {
  }
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// A copy of, or reference to, a real handler object.
#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  typename conditional<
    is_same<CompletionToken, completion_handler_type>::value,
    completion_handler_type&, completion_handler_type>::type completion_handler;
#else // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  completion_handler_type completion_handler;
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// The result of the asynchronous operation's initiating function.
  async_result<typename decay<CompletionToken>::type, Signature> result;
};

namespace detail {

template <typename CompletionToken, typename Signature>
struct async_result_helper
  : async_result<typename decay<CompletionToken>::type, Signature>
{
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

# define ASIO_INITFN_RESULT_TYPE(ct, sig) \
  typename ::asio::async_result< \
    typename ::asio::decay<ct>::type, sig>::return_type
#define ASIO_HANDLER_TYPE(ct, sig) \
  typename ::asio::async_result< \
    typename ::asio::decay<ct>::type, sig>::completion_handler_type

#endif // ASIO_ASYNC_RESULT_HPP
