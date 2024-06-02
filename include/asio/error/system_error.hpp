#ifndef ASIO_SYSTEM_ERROR_HPP
#define ASIO_SYSTEM_ERROR_HPP

#include "asio/detail/config.hpp"

# include <system_error>

#include "asio/detail/push_options.hpp"

namespace asio {

typedef std::system_error system_error;


} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_SYSTEM_ERROR_HPP
