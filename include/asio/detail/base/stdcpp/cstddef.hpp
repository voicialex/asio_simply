#ifndef ASIO_DETAIL_CSTDDEF_HPP
#define ASIO_DETAIL_CSTDDEF_HPP

#include "asio/detail/config.hpp"
#include <cstddef>

namespace asio {

#if defined(ASIO_HAS_NULLPTR)
using std::nullptr_t;
#else // defined(ASIO_HAS_NULLPTR)
struct nullptr_t {};
#endif // defined(ASIO_HAS_NULLPTR)

} // namespace asio

#endif // ASIO_DETAIL_CSTDDEF_HPP
