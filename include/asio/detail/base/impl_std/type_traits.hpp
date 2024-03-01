#ifndef ASIO_DETAIL_TYPE_TRAITS_HPP
#define ASIO_DETAIL_TYPE_TRAITS_HPP

#include "asio/detail/config.hpp"

# include <type_traits>

namespace asio {

using std::add_const;
using std::conditional;
using std::decay;
using std::enable_if;
using std::false_type;
using std::integral_constant;
using std::is_base_of;
using std::is_class;
using std::is_const;
using std::is_convertible;
using std::is_function;
using std::is_same;
using std::remove_pointer;
using std::remove_reference;
#if defined(ASIO_HAS_STD_INVOKE_RESULT)
template <typename> struct result_of;
template <typename F, typename... Args>
struct result_of<F(Args...)> : std::invoke_result<F, Args...> {};
#else // defined(ASIO_HAS_STD_INVOKE_RESULT)
using std::result_of;
#endif // defined(ASIO_HAS_STD_INVOKE_RESULT)
using std::true_type;

} // namespace asio

#endif // ASIO_DETAIL_TYPE_TRAITS_HPP
