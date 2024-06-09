//
// placeholders.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_PLACEHOLDERS_HPP
#define ASIO_PLACEHOLDERS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/base/stdcpp/functional.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace placeholders {

static constexpr auto& error = std::placeholders::_1;
static constexpr auto& bytes_transferred = std::placeholders::_2;
static constexpr auto& iterator = std::placeholders::_2;
static constexpr auto& results = std::placeholders::_2;
static constexpr auto& endpoint = std::placeholders::_2;
static constexpr auto& signal_number = std::placeholders::_2;

} // namespace placeholders
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_PLACEHOLDERS_HPP
