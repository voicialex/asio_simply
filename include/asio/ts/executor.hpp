//
// ts/executor.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_TS_EXECUTOR_HPP
#define ASIO_TS_EXECUTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/handler_type.hpp"
#include "asio/executor/async_result.hpp"
#include "asio/detail/memory/associated_allocator.hpp"
#include "asio/executor/execution_context.hpp"
#include "asio/executor/is_executor.hpp"
#include "asio/executor/associated_executor.hpp"
#include "asio/executor/bind_executor.hpp"
#include "asio/executor/executor_work_guard.hpp"
#include "asio/executor/system_executor.hpp"
#include "asio/executor.hpp"
#include "asio/executor/submit/dispatch.hpp"
#include "asio/executor/submit/post.hpp"
#include "asio/executor/submit/defer.hpp"
// #include "asio/strand.hpp"
// #include "asio/packaged_task.hpp"
// #include "asio/use_future.hpp"

#endif // ASIO_TS_EXECUTOR_HPP