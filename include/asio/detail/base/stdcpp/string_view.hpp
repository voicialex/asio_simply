#ifndef ASIO_DETAIL_STRING_VIEW_HPP
#define ASIO_DETAIL_STRING_VIEW_HPP

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_STRING_VIEW)

# include <string_view>

namespace asio {

using std::basic_string_view;
using std::string_view;

} // namespace asio

# define ASIO_STRING_VIEW_PARAM asio::string_view
#else // defined(ASIO_HAS_STRING_VIEW)
# define ASIO_STRING_VIEW_PARAM const std::string&
#endif // defined(ASIO_HAS_STRING_VIEW)

#endif // ASIO_DETAIL_STRING_VIEW_HPP
