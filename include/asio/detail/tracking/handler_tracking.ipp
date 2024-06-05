#ifndef ASIO_DETAIL_IMPL_HANDLER_TRACKING_IPP
#define ASIO_DETAIL_IMPL_HANDLER_TRACKING_IPP

#include "asio/detail/config.hpp"

#if defined(ASIO_CUSTOM_HANDLER_TRACKING)

// The handler tracking implementation is provided by the user-specified header.

#elif defined(ASIO_ENABLE_HANDLER_TRACKING)

#include "asio/detail/tracking/handler_tracking.hpp"
#include <cstdarg>
#include <cstdio>

#include "asio/timer/chrono_time_traits.hpp"
#include "asio/detail/base/stdcpp/chrono.hpp"
#include "asio/timer/wait_traits.hpp"

#include <unistd.h>

#include "asio/detail/push_options.hpp"

// #define ASIO_HANDLER_TRACK_ENABLE_TIMESTAMP
#ifdef ASIO_HANDLER_TRACK_ENABLE_TIMESTAMP
#define ASIO_HANDLER_TRACK_TAG_KEY "@asio|%llu.%06llu"
#define ASIO_HANDLER_TRACK_TAG_VAL(timestamp) timestamp.seconds, timestamp.microseconds,
#else
#define ASIO_HANDLER_TRACK_TAG_KEY
#define ASIO_HANDLER_TRACK_TAG_VAL(timestamp)
#endif

namespace asio
{
namespace detail
{

struct handler_tracking_timestamp
{
    uint64_t seconds;
    uint64_t microseconds;

    handler_tracking_timestamp()
    {
        typedef chrono_time_traits<chrono::system_clock, asio::wait_traits<chrono::system_clock>> traits_helper;
        traits_helper::posix_time_duration now(chrono::system_clock::now().time_since_epoch());

        seconds = static_cast<uint64_t>(now.total_seconds());
        microseconds = static_cast<uint64_t>(now.total_microseconds() % 1000000);
    }
};

struct handler_tracking::tracking_state
{
    static_mutex mutex_;
    uint64_t next_id_;
    tss_ptr<completion> *current_completion_;
};

handler_tracking::tracking_state *handler_tracking::get_state()
{
    static tracking_state state = {ASIO_STATIC_MUTEX_INIT, 1, 0};
    return &state;
}

void handler_tracking::init()
{
    static tracking_state *state = get_state();

    state->mutex_.init();

    static_mutex::scoped_lock lock(state->mutex_);
    if (state->current_completion_ == 0)
        state->current_completion_ = new tss_ptr<completion>;
}

void handler_tracking::creation(execution_context &, handler_tracking::tracked_handler &h, const char *object_type,
                                void *object, uintmax_t /*native_handle*/, const char *op_name)
{
    static tracking_state *state = get_state();

    static_mutex::scoped_lock lock(state->mutex_);
    h.id_ = state->next_id_++;
    lock.unlock();

    handler_tracking_timestamp timestamp;

    uint64_t current_id = 0;
    if (completion *current_completion = *state->current_completion_)
        current_id = current_completion->id_;

    write_line(ASIO_HANDLER_TRACK_TAG_KEY "|%llu*%llu|%.20s@%p.%.50s\n", ASIO_HANDLER_TRACK_TAG_VAL(timestamp) current_id,
               h.id_, object_type, object, op_name);
}

handler_tracking::completion::completion(const handler_tracking::tracked_handler &h)
    : id_(h.id_), invoked_(false), next_(*get_state()->current_completion_)
{
    *get_state()->current_completion_ = this;
}

handler_tracking::completion::~completion()
{
    if (id_)
    {
        handler_tracking_timestamp timestamp;

        write_line(ASIO_HANDLER_TRACK_TAG_KEY "|%c%llu|\n", ASIO_HANDLER_TRACK_TAG_VAL(timestamp) invoked_ ? '!' : '~',
                   id_);
    }

    *get_state()->current_completion_ = next_;
}

void handler_tracking::completion::invocation_begin()
{
    handler_tracking_timestamp timestamp;

    write_line(ASIO_HANDLER_TRACK_TAG_KEY "|>%llu|\n", ASIO_HANDLER_TRACK_TAG_VAL(timestamp) id_);

    invoked_ = true;
}

void handler_tracking::completion::invocation_begin(const asio::error_code &ec)
{
    handler_tracking_timestamp timestamp;

    write_line(ASIO_HANDLER_TRACK_TAG_KEY "|>%llu|ec=%.20s:%d\n", ASIO_HANDLER_TRACK_TAG_VAL(timestamp) id_,
               ec.category().name(), ec.value());

    invoked_ = true;
}

void handler_tracking::completion::invocation_begin(const asio::error_code &ec, std::size_t bytes_transferred)
{
    handler_tracking_timestamp timestamp;

    write_line(ASIO_HANDLER_TRACK_TAG_KEY "|>%llu|ec=%.20s:%d,bytes_transferred=%llu\n",
               ASIO_HANDLER_TRACK_TAG_VAL(timestamp) id_, ec.category().name(), ec.value(),
               static_cast<uint64_t>(bytes_transferred));

    invoked_ = true;
}

void handler_tracking::completion::invocation_begin(const asio::error_code &ec, int signal_number)
{
    handler_tracking_timestamp timestamp;

    write_line(ASIO_HANDLER_TRACK_TAG_KEY "|>%llu|ec=%.20s:%d,signal_number=%d\n",
               ASIO_HANDLER_TRACK_TAG_VAL(timestamp) id_, ec.category().name(), ec.value(), signal_number);

    invoked_ = true;
}

void handler_tracking::completion::invocation_begin(const asio::error_code &ec, const char *arg)
{
    handler_tracking_timestamp timestamp;

    write_line(ASIO_HANDLER_TRACK_TAG_KEY "|>%llu|ec=%.20s:%d,%.50s\n", ASIO_HANDLER_TRACK_TAG_VAL(timestamp) id_,
               ec.category().name(), ec.value(), arg);

    invoked_ = true;
}

void handler_tracking::completion::invocation_end()
{
    if (id_)
    {
        handler_tracking_timestamp timestamp;

        write_line(ASIO_HANDLER_TRACK_TAG_KEY "|<%llu|\n", ASIO_HANDLER_TRACK_TAG_VAL(timestamp) id_);

        id_ = 0;
    }
}

void handler_tracking::operation(execution_context &, const char *object_type, void *object,
                                 uintmax_t /*native_handle*/, const char *op_name)
{
    static tracking_state *state = get_state();

    handler_tracking_timestamp timestamp;

    unsigned long long current_id = 0;
    if (completion *current_completion = *state->current_completion_)
        current_id = current_completion->id_;

    write_line(ASIO_HANDLER_TRACK_TAG_KEY "|%llu|%.20s@%p.%.50s\n", ASIO_HANDLER_TRACK_TAG_VAL(timestamp) current_id,
               object_type, object, op_name);
}

void handler_tracking::reactor_registration(execution_context & /*context*/, uintmax_t /*native_handle*/,
                                            uintmax_t /*registration*/)
{
}

void handler_tracking::reactor_deregistration(execution_context & /*context*/, uintmax_t /*native_handle*/,
                                              uintmax_t /*registration*/)
{
}

void handler_tracking::reactor_events(execution_context & /*context*/, uintmax_t /*native_handle*/, unsigned /*events*/)
{
}

void handler_tracking::reactor_operation(const tracked_handler &h, const char *op_name, const asio::error_code &ec)
{
    handler_tracking_timestamp timestamp;

    write_line(ASIO_HANDLER_TRACK_TAG_KEY "|.%llu|%s,ec=%.20s:%d\n", ASIO_HANDLER_TRACK_TAG_VAL(timestamp) h.id_,
               op_name, ec.category().name(), ec.value());
}

void handler_tracking::reactor_operation(const tracked_handler &h, const char *op_name, const asio::error_code &ec,
                                         std::size_t bytes_transferred)
{
    handler_tracking_timestamp timestamp;

    write_line(ASIO_HANDLER_TRACK_TAG_KEY "|.%llu|%s,ec=%.20s:%d,bytes_transferred=%llu\n",
               ASIO_HANDLER_TRACK_TAG_VAL(timestamp) h.id_, op_name, ec.category().name(), ec.value(),
               static_cast<uint64_t>(bytes_transferred));
}

void handler_tracking::write_line(const char *format, ...)
{
    using namespace std; // For sprintf (or equivalent).

    va_list args;
    va_start(args, format);

    char line[256] = "";
#if defined(ASIO_HAS_SECURE_RTL)
    int length = vsprintf_s(line, sizeof(line), format, args);
#else  // defined(ASIO_HAS_SECURE_RTL)
    int length = vsprintf(line, format, args);
#endif // defined(ASIO_HAS_SECURE_RTL)

    va_end(args);

    ::write(STDERR_FILENO, line, length);
}

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_ENABLE_HANDLER_TRACKING)

#endif // ASIO_DETAIL_IMPL_HANDLER_TRACKING_IPP
