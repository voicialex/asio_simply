//
// detail/static_mutex.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2016 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_STATIC_MUTEX_HPP
#define ASIO_DETAIL_STATIC_MUTEX_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"


# include "asio/detail/base/posix/posix_static_mutex.hpp"


namespace asio {
namespace detail {

typedef posix_static_mutex static_mutex;
# define ASIO_STATIC_MUTEX_INIT ASIO_POSIX_STATIC_MUTEX_INIT

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_STATIC_MUTEX_HPP
