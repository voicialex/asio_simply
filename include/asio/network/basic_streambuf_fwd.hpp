#ifndef ASIO_BASIC_STREAMBUF_FWD_HPP
#define ASIO_BASIC_STREAMBUF_FWD_HPP

#include "asio/detail/config.hpp"

#if !defined(ASIO_NO_IOSTREAM)

#include <memory>

namespace asio {

template <typename Allocator = std::allocator<char> >
class basic_streambuf;

template <typename Allocator = std::allocator<char> >
class basic_streambuf_ref;

} // namespace asio

#endif // !defined(ASIO_NO_IOSTREAM)

#endif // ASIO_BASIC_STREAMBUF_FWD_HPP
