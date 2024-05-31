#ifndef ASIO_DETAIL_SOCKET_OPS_IPP
#define ASIO_DETAIL_SOCKET_OPS_IPP

#include "asio/detail/config.hpp"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <new>
#include "asio/detail/base/stdcpp/assert.hpp"
#include "asio/detail/socket_ops.hpp"
#include "asio/error.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {
namespace socket_ops {

inline void clear_last_error()
{
  errno = 0;
}

#if !defined(ASIO_WINDOWS_RUNTIME)

template <typename ReturnType>
inline ReturnType error_wrapper(ReturnType return_value,
    asio::error_code& ec)
{
  ec = asio::error_code(errno,
      asio::error::get_system_category());
  return return_value;
}

template <typename SockLenType>
inline socket_type call_accept(SockLenType msghdr::*,
    socket_type s, socket_addr_type* addr, std::size_t* addrlen)
{
  SockLenType tmp_addrlen = addrlen ? (SockLenType)*addrlen : 0;
  socket_type result = ::accept(s, addr, addrlen ? &tmp_addrlen : 0);
  if (addrlen)
    *addrlen = (std::size_t)tmp_addrlen;
  return result;
}

socket_type accept(socket_type s, socket_addr_type* addr,
    std::size_t* addrlen, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return invalid_socket;
  }

  clear_last_error();

  socket_type new_s = error_wrapper(call_accept(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
  if (new_s == invalid_socket)
    return new_s;

#if defined(__MACH__) && defined(__APPLE__) || defined(__FreeBSD__)
  int optval = 1;
  int result = error_wrapper(::setsockopt(new_s,
        SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval)), ec);
  if (result != 0)
  {
    ::close(new_s);
    return invalid_socket;
  }
#endif

  ec = asio::error_code();
  return new_s;
}

socket_type sync_accept(socket_type s, state_type state,
    socket_addr_type* addr, std::size_t* addrlen, asio::error_code& ec)
{
  // Accept a socket.
  for (;;)
  {
    // Try to complete the operation without blocking.
    socket_type new_socket = socket_ops::accept(s, addr, addrlen, ec);

    // Check if operation succeeded.
    if (new_socket != invalid_socket)
      return new_socket;

    // Operation failed.
    if (ec == asio::error::would_block
        || ec == asio::error::try_again)
    {
      if (state & user_set_non_blocking)
        return invalid_socket;
      // Fall through to retry operation.
    }
    else if (ec == asio::error::connection_aborted)
    {
      if (state & enable_connection_aborted)
        return invalid_socket;
      // Fall through to retry operation.
    }
#if defined(EPROTO)
    else if (ec.value() == EPROTO)
    {
      if (state & enable_connection_aborted)
        return invalid_socket;
      // Fall through to retry operation.
    }
#endif // defined(EPROTO)
    else
      return invalid_socket;

    // Wait for socket to become ready.
    if (socket_ops::poll_read(s, 0, -1, ec) < 0)
      return invalid_socket;
  }
}

bool non_blocking_accept(socket_type s,
    state_type state, socket_addr_type* addr, std::size_t* addrlen,
    asio::error_code& ec, socket_type& new_socket)
{
  for (;;)
  {
    // Accept the waiting connection.
    new_socket = socket_ops::accept(s, addr, addrlen, ec);

    // Check if operation succeeded.
    if (new_socket != invalid_socket)
      return true;

    // Retry operation if interrupted by signal.
    if (ec == asio::error::interrupted)
      continue;

    // Operation failed.
    if (ec == asio::error::would_block
        || ec == asio::error::try_again)
    {
      // Fall through to retry operation.
    }
    else if (ec == asio::error::connection_aborted)
    {
      if (state & enable_connection_aborted)
        return true;
      // Fall through to retry operation.
    }
#if defined(EPROTO)
    else if (ec.value() == EPROTO)
    {
      if (state & enable_connection_aborted)
        return true;
      // Fall through to retry operation.
    }
#endif // defined(EPROTO)
    else
      return true;

    return false;
  }
}

template <typename SockLenType>
inline int call_bind(SockLenType msghdr::*,
    socket_type s, const socket_addr_type* addr, std::size_t addrlen)
{
  return ::bind(s, addr, (SockLenType)addrlen);
}

int bind(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return socket_error_retval;
  }

  clear_last_error();
  int result = error_wrapper(call_bind(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
  if (result == 0)
    ec = asio::error_code();
  return result;
}

int close(socket_type s, state_type& state,
    bool destruction, asio::error_code& ec)
{
  int result = 0;
  if (s != invalid_socket)
  {
    // We don't want the destructor to block, so set the socket to linger in
    // the background. If the user doesn't like this behaviour then they need
    // to explicitly close the socket.
    if (destruction && (state & user_set_linger))
    {
      ::linger opt;
      opt.l_onoff = 0;
      opt.l_linger = 0;
      asio::error_code ignored_ec;
      socket_ops::setsockopt(s, state, SOL_SOCKET,
          SO_LINGER, &opt, sizeof(opt), ignored_ec);
    }

    clear_last_error();
    result = error_wrapper(::close(s), ec);

    if (result != 0
        && (ec == asio::error::would_block
          || ec == asio::error::try_again))
    {
      // According to UNIX Network Programming Vol. 1, it is possible for
      // close() to fail with EWOULDBLOCK under certain circumstances. What
      // isn't clear is the state of the descriptor after this error. The one
      // current OS where this behaviour is seen, Windows, says that the socket
      // remains open. Therefore we'll put the descriptor back into blocking
      // mode and have another attempt at closing it.

      ioctl_arg_type arg = 0;
      ::ioctl(s, FIONBIO, &arg);

      state &= ~non_blocking;

      clear_last_error();

      result = error_wrapper(::close(s), ec);
    }
  }

  if (result == 0)
    ec = asio::error_code();
  return result;
}

bool set_user_non_blocking(socket_type s,
    state_type& state, bool value, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return false;
  }

  clear_last_error();

  ioctl_arg_type arg = (value ? 1 : 0);
  int result = error_wrapper(::ioctl(s, FIONBIO, &arg), ec);

  if (result >= 0)
  {
    ec = asio::error_code();
    if (value)
      state |= user_set_non_blocking;
    else
    {
      // Clearing the user-set non-blocking mode always overrides any
      // internally-set non-blocking flag. Any subsequent asynchronous
      // operations will need to re-enable non-blocking I/O.
      state &= ~(user_set_non_blocking | internal_non_blocking);
    }
    return true;
  }

  return false;
}

bool set_internal_non_blocking(socket_type s,
    state_type& state, bool value, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return false;
  }

  if (!value && (state & user_set_non_blocking))
  {
    // It does not make sense to clear the internal non-blocking flag if the
    // user still wants non-blocking behaviour. Return an error and let the
    // caller figure out whether to update the user-set non-blocking flag.
    ec = asio::error::invalid_argument;
    return false;
  }

  clear_last_error();

  ioctl_arg_type arg = (value ? 1 : 0);
  int result = error_wrapper(::ioctl(s, FIONBIO, &arg), ec);

  if (result >= 0)
  {
    ec = asio::error_code();
    if (value)
      state |= internal_non_blocking;
    else
      state &= ~internal_non_blocking;
    return true;
  }

  return false;
}

int shutdown(socket_type s, int what, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return socket_error_retval;
  }

  clear_last_error();
  int result = error_wrapper(::shutdown(s, what), ec);
  if (result == 0)
    ec = asio::error_code();
  return result;
}

template <typename SockLenType>
inline int call_connect(SockLenType msghdr::*,
    socket_type s, const socket_addr_type* addr, std::size_t addrlen)
{
  return ::connect(s, addr, (SockLenType)addrlen);
}

int connect(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return socket_error_retval;
  }

  clear_last_error();
  int result = error_wrapper(call_connect(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
  if (result == 0)
    ec = asio::error_code();
#if defined(__linux__)
  else if (ec == asio::error::try_again)
    ec = asio::error::no_buffer_space;
#endif // defined(__linux__)
  return result;
}

void sync_connect(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, asio::error_code& ec)
{
  // Perform the connect operation.
  socket_ops::connect(s, addr, addrlen, ec);
  if (ec != asio::error::in_progress
      && ec != asio::error::would_block)
  {
    // The connect operation finished immediately.
    return;
  }

  // Wait for socket to become ready.
  if (socket_ops::poll_connect(s, -1, ec) < 0)
    return;

  // Get the error code from the connect operation.
  int connect_error = 0;
  size_t connect_error_len = sizeof(connect_error);
  if (socket_ops::getsockopt(s, 0, SOL_SOCKET, SO_ERROR,
        &connect_error, &connect_error_len, ec) == socket_error_retval)
    return;

  // Return the result of the connect operation.
  ec = asio::error_code(connect_error,
      asio::error::get_system_category());
}

bool non_blocking_connect(socket_type s, asio::error_code& ec)
{
  // Check if the connect operation has finished. This is required since we may
  // get spurious readiness notifications from the reactor.

  pollfd fds;
  fds.fd = s;
  fds.events = POLLOUT;
  fds.revents = 0;
  int ready = ::poll(&fds, 1, 0);
  if (ready == 0)
  {
    // The asynchronous connect operation is still in progress.
    return false;
  }

  // Get the error code from the connect operation.
  int connect_error = 0;
  size_t connect_error_len = sizeof(connect_error);
  if (socket_ops::getsockopt(s, 0, SOL_SOCKET, SO_ERROR,
        &connect_error, &connect_error_len, ec) == 0)
  {
    if (connect_error)
    {
      ec = asio::error_code(connect_error,
          asio::error::get_system_category());
    }
    else
      ec = asio::error_code();
  }

  return true;
}

int socketpair(int af, int type, int protocol,
    socket_type sv[2], asio::error_code& ec)
{
  clear_last_error();
  int result = error_wrapper(::socketpair(af, type, protocol, sv), ec);
  if (result == 0)
    ec = asio::error_code();
  return result;
}

bool sockatmark(socket_type s, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return false;
  }

#if defined(SIOCATMARK)
  ioctl_arg_type value = 0;

  int result = error_wrapper(::ioctl(s, SIOCATMARK, &value), ec);

  if (result == 0)
    ec = asio::error_code();
# if defined(ENOTTY)
  if (ec.value() == ENOTTY)
    ec = asio::error::not_socket;
# endif // defined(ENOTTY)
#else // defined(SIOCATMARK)
  int value = error_wrapper(::sockatmark(s), ec);
  if (value != -1)
    ec = asio::error_code();
#endif // defined(SIOCATMARK)

  return ec ? false : value != 0;
}

size_t available(socket_type s, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return 0;
  }

  ioctl_arg_type value = 0;

  int result = error_wrapper(::ioctl(s, FIONREAD, &value), ec);

  if (result == 0)
    ec = asio::error_code();
#if defined(ENOTTY)
  if (ec.value() == ENOTTY)
    ec = asio::error::not_socket;
#endif // defined(ENOTTY)

  return ec ? static_cast<size_t>(0) : static_cast<size_t>(value);
}

int listen(socket_type s, int backlog, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return socket_error_retval;
  }

  clear_last_error();
  int result = error_wrapper(::listen(s, backlog), ec);
  if (result == 0)
    ec = asio::error_code();
  return result;
}

inline void init_buf_iov_base(void*& base, void* addr)
{
  base = addr;
}

template <typename T>
inline void init_buf_iov_base(T& base, void* addr)
{
  base = static_cast<T>(addr);
}

typedef iovec buf;

void init_buf(buf& b, void* data, size_t size)
{
  init_buf_iov_base(b.iov_base, data);
  b.iov_len = size;
}

void init_buf(buf& b, const void* data, size_t size)
{
  init_buf_iov_base(b.iov_base, const_cast<void*>(data));
  b.iov_len = size;
}

inline void init_msghdr_msg_name(void*& name, socket_addr_type* addr)
{
  name = addr;
}

inline void init_msghdr_msg_name(void*& name, const socket_addr_type* addr)
{
  name = const_cast<socket_addr_type*>(addr);
}

template <typename T>
inline void init_msghdr_msg_name(T& name, socket_addr_type* addr)
{
  name = reinterpret_cast<T>(addr);
}

template <typename T>
inline void init_msghdr_msg_name(T& name, const socket_addr_type* addr)
{
  name = reinterpret_cast<T>(const_cast<socket_addr_type*>(addr));
}

signed_size_type recv(socket_type s, buf* bufs, size_t count,
    int flags, asio::error_code& ec)
{
  clear_last_error();
  msghdr msg = msghdr();
  msg.msg_iov = bufs;
  msg.msg_iovlen = static_cast<int>(count);
  signed_size_type result = error_wrapper(::recvmsg(s, &msg, flags), ec);
  if (result >= 0)
    ec = asio::error_code();
  return result;
}

size_t sync_recv(socket_type s, state_type state, buf* bufs,
    size_t count, int flags, bool all_empty, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return 0;
  }

  // A request to read 0 bytes on a stream is a no-op.
  if (all_empty && (state & stream_oriented))
  {
    ec = asio::error_code();
    return 0;
  }

  // Read some data.
  for (;;)
  {
    // Try to complete the operation without blocking.
    signed_size_type bytes = socket_ops::recv(s, bufs, count, flags, ec);

    // Check if operation succeeded.
    if (bytes > 0)
      return bytes;

    // Check for EOF.
    if ((state & stream_oriented) && bytes == 0)
    {
      ec = asio::error::eof;
      return 0;
    }

    // Operation failed.
    if ((state & user_set_non_blocking)
        || (ec != asio::error::would_block
          && ec != asio::error::try_again))
      return 0;

    // Wait for socket to become ready.
    if (socket_ops::poll_read(s, 0, -1, ec) < 0)
      return 0;
  }
}

bool non_blocking_recv(socket_type s,
    buf* bufs, size_t count, int flags, bool is_stream,
    asio::error_code& ec, size_t& bytes_transferred)
{
  for (;;)
  {
    // Read some data.
    signed_size_type bytes = socket_ops::recv(s, bufs, count, flags, ec);

    // Check for end of stream.
    if (is_stream && bytes == 0)
    {
      ec = asio::error::eof;
      return true;
    }

    // Retry operation if interrupted by signal.
    if (ec == asio::error::interrupted)
      continue;

    // Check if we need to run the operation again.
    if (ec == asio::error::would_block
        || ec == asio::error::try_again)
      return false;

    // Operation is complete.
    if (bytes >= 0)
    {
      ec = asio::error_code();
      bytes_transferred = bytes;
    }
    else
      bytes_transferred = 0;

    return true;
  }
}

signed_size_type recvfrom(socket_type s, buf* bufs, size_t count,
    int flags, socket_addr_type* addr, std::size_t* addrlen,
    asio::error_code& ec)
{
  clear_last_error();
  msghdr msg = msghdr();
  init_msghdr_msg_name(msg.msg_name, addr);
  msg.msg_namelen = static_cast<int>(*addrlen);
  msg.msg_iov = bufs;
  msg.msg_iovlen = static_cast<int>(count);
  signed_size_type result = error_wrapper(::recvmsg(s, &msg, flags), ec);
  *addrlen = msg.msg_namelen;
  if (result >= 0)
    ec = asio::error_code();
  return result;
}

size_t sync_recvfrom(socket_type s, state_type state, buf* bufs,
    size_t count, int flags, socket_addr_type* addr,
    std::size_t* addrlen, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return 0;
  }

  // Read some data.
  for (;;)
  {
    // Try to complete the operation without blocking.
    signed_size_type bytes = socket_ops::recvfrom(
        s, bufs, count, flags, addr, addrlen, ec);

    // Check if operation succeeded.
    if (bytes >= 0)
      return bytes;

    // Operation failed.
    if ((state & user_set_non_blocking)
        || (ec != asio::error::would_block
          && ec != asio::error::try_again))
      return 0;

    // Wait for socket to become ready.
    if (socket_ops::poll_read(s, 0, -1, ec) < 0)
      return 0;
  }
}

bool non_blocking_recvfrom(socket_type s,
    buf* bufs, size_t count, int flags,
    socket_addr_type* addr, std::size_t* addrlen,
    asio::error_code& ec, size_t& bytes_transferred)
{
  for (;;)
  {
    // Read some data.
    signed_size_type bytes = socket_ops::recvfrom(
        s, bufs, count, flags, addr, addrlen, ec);

    // Retry operation if interrupted by signal.
    if (ec == asio::error::interrupted)
      continue;

    // Check if we need to run the operation again.
    if (ec == asio::error::would_block
        || ec == asio::error::try_again)
      return false;

    // Operation is complete.
    if (bytes >= 0)
    {
      ec = asio::error_code();
      bytes_transferred = bytes;
    }
    else
      bytes_transferred = 0;

    return true;
  }
}

signed_size_type recvmsg(socket_type s, buf* bufs, size_t count,
    int in_flags, int& out_flags, asio::error_code& ec)
{
  clear_last_error();
  msghdr msg = msghdr();
  msg.msg_iov = bufs;
  msg.msg_iovlen = static_cast<int>(count);
  signed_size_type result = error_wrapper(::recvmsg(s, &msg, in_flags), ec);
  if (result >= 0)
  {
    ec = asio::error_code();
    out_flags = msg.msg_flags;
  }
  else
    out_flags = 0;
  return result;
}

size_t sync_recvmsg(socket_type s, state_type state,
    buf* bufs, size_t count, int in_flags, int& out_flags,
    asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return 0;
  }

  // Read some data.
  for (;;)
  {
    // Try to complete the operation without blocking.
    signed_size_type bytes = socket_ops::recvmsg(
        s, bufs, count, in_flags, out_flags, ec);

    // Check if operation succeeded.
    if (bytes >= 0)
      return bytes;

    // Operation failed.
    if ((state & user_set_non_blocking)
        || (ec != asio::error::would_block
          && ec != asio::error::try_again))
      return 0;

    // Wait for socket to become ready.
    if (socket_ops::poll_read(s, 0, -1, ec) < 0)
      return 0;
  }
}

bool non_blocking_recvmsg(socket_type s,
    buf* bufs, size_t count, int in_flags, int& out_flags,
    asio::error_code& ec, size_t& bytes_transferred)
{
  for (;;)
  {
    // Read some data.
    signed_size_type bytes = socket_ops::recvmsg(
        s, bufs, count, in_flags, out_flags, ec);

    // Retry operation if interrupted by signal.
    if (ec == asio::error::interrupted)
      continue;

    // Check if we need to run the operation again.
    if (ec == asio::error::would_block
        || ec == asio::error::try_again)
      return false;

    // Operation is complete.
    if (bytes >= 0)
    {
      ec = asio::error_code();
      bytes_transferred = bytes;
    }
    else
      bytes_transferred = 0;

    return true;
  }
}

signed_size_type send(socket_type s, const buf* bufs, size_t count,
    int flags, asio::error_code& ec)
{
  clear_last_error();
  msghdr msg = msghdr();
  msg.msg_iov = const_cast<buf*>(bufs);
  msg.msg_iovlen = static_cast<int>(count);
#if defined(__linux__)
  flags |= MSG_NOSIGNAL;
#endif // defined(__linux__)
  signed_size_type result = error_wrapper(::sendmsg(s, &msg, flags), ec);
  if (result >= 0)
    ec = asio::error_code();
  return result;
}

size_t sync_send(socket_type s, state_type state, const buf* bufs,
    size_t count, int flags, bool all_empty, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return 0;
  }

  // A request to write 0 bytes to a stream is a no-op.
  if (all_empty && (state & stream_oriented))
  {
    ec = asio::error_code();
    return 0;
  }

  // Read some data.
  for (;;)
  {
    // Try to complete the operation without blocking.
    signed_size_type bytes = socket_ops::send(s, bufs, count, flags, ec);

    // Check if operation succeeded.
    if (bytes >= 0)
      return bytes;

    // Operation failed.
    if ((state & user_set_non_blocking)
        || (ec != asio::error::would_block
          && ec != asio::error::try_again))
      return 0;

    // Wait for socket to become ready.
    if (socket_ops::poll_write(s, 0, -1, ec) < 0)
      return 0;
  }
}

bool non_blocking_send(socket_type s,
    const buf* bufs, size_t count, int flags,
    asio::error_code& ec, size_t& bytes_transferred)
{
  for (;;)
  {
    // Write some data.
    signed_size_type bytes = socket_ops::send(s, bufs, count, flags, ec);

    // Retry operation if interrupted by signal.
    if (ec == asio::error::interrupted)
      continue;

    // Check if we need to run the operation again.
    if (ec == asio::error::would_block
        || ec == asio::error::try_again)
      return false;

    // Operation is complete.
    if (bytes >= 0)
    {
      ec = asio::error_code();
      bytes_transferred = bytes;
    }
    else
      bytes_transferred = 0;

    return true;
  }
}

signed_size_type sendto(socket_type s, const buf* bufs, size_t count,
    int flags, const socket_addr_type* addr, std::size_t addrlen,
    asio::error_code& ec)
{
  clear_last_error();
  msghdr msg = msghdr();
  init_msghdr_msg_name(msg.msg_name, addr);
  msg.msg_namelen = static_cast<int>(addrlen);
  msg.msg_iov = const_cast<buf*>(bufs);
  msg.msg_iovlen = static_cast<int>(count);
#if defined(__linux__)
  flags |= MSG_NOSIGNAL;
#endif // defined(__linux__)
  signed_size_type result = error_wrapper(::sendmsg(s, &msg, flags), ec);
  if (result >= 0)
    ec = asio::error_code();
  return result;
}

size_t sync_sendto(socket_type s, state_type state, const buf* bufs,
    size_t count, int flags, const socket_addr_type* addr,
    std::size_t addrlen, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return 0;
  }

  // Write some data.
  for (;;)
  {
    // Try to complete the operation without blocking.
    signed_size_type bytes = socket_ops::sendto(
        s, bufs, count, flags, addr, addrlen, ec);

    // Check if operation succeeded.
    if (bytes >= 0)
      return bytes;

    // Operation failed.
    if ((state & user_set_non_blocking)
        || (ec != asio::error::would_block
          && ec != asio::error::try_again))
      return 0;

    // Wait for socket to become ready.
    if (socket_ops::poll_write(s, 0, -1, ec) < 0)
      return 0;
  }
}

socket_type socket(int af, int type, int protocol,
    asio::error_code& ec)
{
  clear_last_error();
  int s = error_wrapper(::socket(af, type, protocol), ec);
  if (s >= 0)
    ec = asio::error_code();
  return s;
}

template <typename SockLenType>
inline int call_setsockopt(SockLenType msghdr::*,
    socket_type s, int level, int optname,
    const void* optval, std::size_t optlen)
{
  return ::setsockopt(s, level, optname,
      (const char*)optval, (SockLenType)optlen);
}

int setsockopt(socket_type s, state_type& state, int level, int optname,
    const void* optval, std::size_t optlen, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return socket_error_retval;
  }

  if (level == custom_socket_option_level && optname == always_fail_option)
  {
    ec = asio::error::invalid_argument;
    return socket_error_retval;
  }

  if (level == custom_socket_option_level
      && optname == enable_connection_aborted_option)
  {
    if (optlen != sizeof(int))
    {
      ec = asio::error::invalid_argument;
      return socket_error_retval;
    }

    if (*static_cast<const int*>(optval))
      state |= enable_connection_aborted;
    else
      state &= ~enable_connection_aborted;
    ec = asio::error_code();
    return 0;
  }

  if (level == SOL_SOCKET && optname == SO_LINGER)
    state |= user_set_linger;

#if defined(__BORLANDC__)
  // Mysteriously, using the getsockopt and setsockopt functions directly with
  // Borland C++ results in incorrect values being set and read. The bug can be
  // worked around by using function addresses resolved with GetProcAddress.
  if (HMODULE winsock_module = ::GetModuleHandleA("ws2_32"))
  {
    typedef int (WSAAPI *sso_t)(SOCKET, int, int, const char*, int);
    if (sso_t sso = (sso_t)::GetProcAddress(winsock_module, "setsockopt"))
    {
      clear_last_error();
      return error_wrapper(sso(s, level, optname,
            reinterpret_cast<const char*>(optval),
            static_cast<int>(optlen)), ec);
    }
  }
  ec = asio::error::fault;
  return socket_error_retval;
#else // defined(__BORLANDC__)
  clear_last_error();
  int result = error_wrapper(call_setsockopt(&msghdr::msg_namelen,
        s, level, optname, optval, optlen), ec);
  if (result == 0)
  {
    ec = asio::error_code();

#if defined(__MACH__) && defined(__APPLE__) \
  || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
    // To implement portable behaviour for SO_REUSEADDR with UDP sockets we
    // need to also set SO_REUSEPORT on BSD-based platforms.
    if ((state & datagram_oriented)
        && level == SOL_SOCKET && optname == SO_REUSEADDR)
    {
      call_setsockopt(&msghdr::msg_namelen, s,
          SOL_SOCKET, SO_REUSEPORT, optval, optlen);
    }
#endif
  }

  return result;
#endif // defined(__BORLANDC__)
}

template <typename SockLenType>
inline int call_getsockopt(SockLenType msghdr::*,
    socket_type s, int level, int optname,
    void* optval, std::size_t* optlen)
{
  SockLenType tmp_optlen = (SockLenType)*optlen;
  int result = ::getsockopt(s, level, optname, (char*)optval, &tmp_optlen);
  *optlen = (std::size_t)tmp_optlen;
  return result;
}

int getsockopt(socket_type s, state_type state, int level, int optname,
    void* optval, size_t* optlen, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return socket_error_retval;
  }

  if (level == custom_socket_option_level && optname == always_fail_option)
  {
    ec = asio::error::invalid_argument;
    return socket_error_retval;
  }

  if (level == custom_socket_option_level
      && optname == enable_connection_aborted_option)
  {
    if (*optlen != sizeof(int))
    {
      ec = asio::error::invalid_argument;
      return socket_error_retval;
    }

    *static_cast<int*>(optval) = (state & enable_connection_aborted) ? 1 : 0;
    ec = asio::error_code();
    return 0;
  }

#if defined(__BORLANDC__)
  // Mysteriously, using the getsockopt and setsockopt functions directly with
  // Borland C++ results in incorrect values being set and read. The bug can be
  // worked around by using function addresses resolved with GetProcAddress.
  if (HMODULE winsock_module = ::GetModuleHandleA("ws2_32"))
  {
    typedef int (WSAAPI *gso_t)(SOCKET, int, int, char*, int*);
    if (gso_t gso = (gso_t)::GetProcAddress(winsock_module, "getsockopt"))
    {
      clear_last_error();
      int tmp_optlen = static_cast<int>(*optlen);
      int result = error_wrapper(gso(s, level, optname,
            reinterpret_cast<char*>(optval), &tmp_optlen), ec);
      *optlen = static_cast<size_t>(tmp_optlen);
      if (result != 0 && level == IPPROTO_IPV6 && optname == IPV6_V6ONLY
          && ec.value() == WSAENOPROTOOPT && *optlen == sizeof(DWORD))
      {
        // Dual-stack IPv4/v6 sockets, and the IPV6_V6ONLY socket option, are
        // only supported on Windows Vista and later. To simplify program logic
        // we will fake success of getting this option and specify that the
        // value is non-zero (i.e. true). This corresponds to the behavior of
        // IPv6 sockets on Windows platforms pre-Vista.
        *static_cast<DWORD*>(optval) = 1;
        ec = asio::error_code();
      }
      return result;
    }
  }
  ec = asio::error::fault;
  return socket_error_retval;
#else
  clear_last_error();
  int result = error_wrapper(call_getsockopt(&msghdr::msg_namelen,
        s, level, optname, optval, optlen), ec);
#if defined(__linux__)
  if (result == 0 && level == SOL_SOCKET && *optlen == sizeof(int)
      && (optname == SO_SNDBUF || optname == SO_RCVBUF))
  {
    // On Linux, setting SO_SNDBUF or SO_RCVBUF to N actually causes the kernel
    // to set the buffer size to N*2. Linux puts additional stuff into the
    // buffers so that only about half is actually available to the application.
    // The retrieved value is divided by 2 here to make it appear as though the
    // correct value has been set.
    *static_cast<int*>(optval) /= 2;
  }
#endif // defined(__linux__)
  if (result == 0)
    ec = asio::error_code();
  return result;
#endif
}

template <typename SockLenType>
inline int call_getpeername(SockLenType msghdr::*,
    socket_type s, socket_addr_type* addr, std::size_t* addrlen)
{
  SockLenType tmp_addrlen = (SockLenType)*addrlen;
  int result = ::getpeername(s, addr, &tmp_addrlen);
  *addrlen = (std::size_t)tmp_addrlen;
  return result;
}

int getpeername(socket_type s, socket_addr_type* addr,
    std::size_t* addrlen, bool cached, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return socket_error_retval;
  }

  (void)cached;

  clear_last_error();
  int result = error_wrapper(call_getpeername(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
  if (result == 0)
    ec = asio::error_code();
  return result;
}

template <typename SockLenType>
inline int call_getsockname(SockLenType msghdr::*,
    socket_type s, socket_addr_type* addr, std::size_t* addrlen)
{
  SockLenType tmp_addrlen = (SockLenType)*addrlen;
  int result = ::getsockname(s, addr, &tmp_addrlen);
  *addrlen = (std::size_t)tmp_addrlen;
  return result;
}

int getsockname(socket_type s, socket_addr_type* addr,
    std::size_t* addrlen, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return socket_error_retval;
  }

  clear_last_error();
  int result = error_wrapper(call_getsockname(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
  if (result == 0)
    ec = asio::error_code();
  return result;
}

int ioctl(socket_type s, state_type& state, int cmd,
    ioctl_arg_type* arg, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return socket_error_retval;
  }

  clear_last_error();

  int result = error_wrapper(::ioctl(s, cmd, arg), ec);

  if (result >= 0)
  {
    ec = asio::error_code();

    // When updating the non-blocking mode we always perform the ioctl syscall,
    // even if the flags would otherwise indicate that the socket is already in
    // the correct state. This ensures that the underlying socket is put into
    // the state that has been requested by the user. If the ioctl syscall was
    // successful then we need to update the flags to match.
    if (cmd == static_cast<int>(FIONBIO))
    {
      if (*arg)
      {
        state |= user_set_non_blocking;
      }
      else
      {
        // Clearing the non-blocking mode always overrides any internally-set
        // non-blocking flag. Any subsequent asynchronous operations will need
        // to re-enable non-blocking I/O.
        state &= ~(user_set_non_blocking | internal_non_blocking);
      }
    }
  }

  return result;
}

int select(int nfds, fd_set* readfds, fd_set* writefds,
    fd_set* exceptfds, timeval* timeout, asio::error_code& ec)
{
  clear_last_error();

#if defined(__hpux) && defined(__SELECT)
  timespec ts;
  ts.tv_sec = timeout ? timeout->tv_sec : 0;
  ts.tv_nsec = timeout ? timeout->tv_usec * 1000 : 0;
  return error_wrapper(::pselect(nfds, readfds,
        writefds, exceptfds, timeout ? &ts : 0, 0), ec);
#else
  int result = error_wrapper(::select(nfds, readfds,
        writefds, exceptfds, timeout), ec);
  if (result >= 0)
    ec = asio::error_code();
  return result;
#endif
}

int poll_read(socket_type s, state_type state,
    int msec, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return socket_error_retval;
  }

  pollfd fds;
  fds.fd = s;
  fds.events = POLLIN;
  fds.revents = 0;
  int timeout = (state & user_set_non_blocking) ? 0 : msec;
  clear_last_error();
  int result = error_wrapper(::poll(&fds, 1, timeout), ec);

  if (result == 0)
    ec = (state & user_set_non_blocking)
      ? asio::error::would_block : asio::error_code();
  else if (result > 0)
    ec = asio::error_code();
  return result;
}

int poll_write(socket_type s, state_type state,
    int msec, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return socket_error_retval;
  }

  pollfd fds;
  fds.fd = s;
  fds.events = POLLOUT;
  fds.revents = 0;
  int timeout = (state & user_set_non_blocking) ? 0 : msec;
  clear_last_error();
  int result = error_wrapper(::poll(&fds, 1, timeout), ec);

  if (result == 0)
    ec = (state & user_set_non_blocking)
      ? asio::error::would_block : asio::error_code();
  else if (result > 0)
    ec = asio::error_code();
  return result;
}

int poll_error(socket_type s, state_type state,
    int msec, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return socket_error_retval;
  }

  pollfd fds;
  fds.fd = s;
  fds.events = POLLPRI | POLLERR | POLLHUP;
  fds.revents = 0;
  int timeout = (state & user_set_non_blocking) ? 0 : msec;
  clear_last_error();
  int result = error_wrapper(::poll(&fds, 1, timeout), ec);

  if (result == 0)
    ec = (state & user_set_non_blocking)
      ? asio::error::would_block : asio::error_code();
  else if (result > 0)
    ec = asio::error_code();
  return result;
}

int poll_connect(socket_type s, int msec, asio::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = asio::error::bad_descriptor;
    return socket_error_retval;
  }

  pollfd fds;
  fds.fd = s;
  fds.events = POLLOUT;
  fds.revents = 0;
  clear_last_error();
  int result = error_wrapper(::poll(&fds, 1, msec), ec);
  if (result >= 0)
    ec = asio::error_code();
  return result;
}

#endif // !defined(ASIO_WINDOWS_RUNTIME)

const char* inet_ntop(int af, const void* src, char* dest, size_t length,
    unsigned long scope_id, asio::error_code& ec)
{
  clear_last_error();

  const char* result = error_wrapper(::inet_ntop(
        af, src, dest, static_cast<int>(length)), ec);
  if (result == 0 && !ec)
    ec = asio::error::invalid_argument;
  if (result != 0 && af == ASIO_OS_DEF(AF_INET6) && scope_id != 0)
  {
    using namespace std; // For strcat and sprintf.
    char if_name[(IF_NAMESIZE > 21 ? IF_NAMESIZE : 21) + 1] = "%";
    const in6_addr_type* ipv6_address = static_cast<const in6_addr_type*>(src);
    bool is_link_local = ((ipv6_address->s6_addr[0] == 0xfe)
        && ((ipv6_address->s6_addr[1] & 0xc0) == 0x80));
    bool is_multicast_link_local = ((ipv6_address->s6_addr[0] == 0xff)
        && ((ipv6_address->s6_addr[1] & 0x0f) == 0x02));
    if ((!is_link_local && !is_multicast_link_local)
        || if_indextoname(static_cast<unsigned>(scope_id), if_name + 1) == 0)
      sprintf(if_name + 1, "%lu", scope_id);
    strcat(dest, if_name);
  }
  return result;
}

int inet_pton(int af, const char* src, void* dest,
    unsigned long* scope_id, asio::error_code& ec)
{
  clear_last_error();

  using namespace std; // For strchr, memcpy and atoi.

  // On some platforms, inet_pton fails if an address string contains a scope
  // id. Detect and remove the scope id before passing the string to inet_pton.
  const bool is_v6 = (af == ASIO_OS_DEF(AF_INET6));
  const char* if_name = is_v6 ? strchr(src, '%') : 0;
  char src_buf[max_addr_v6_str_len + 1];
  const char* src_ptr = src;
  if (if_name != 0)
  {
    if (if_name - src > max_addr_v6_str_len)
    {
      ec = asio::error::invalid_argument;
      return 0;
    }
    memcpy(src_buf, src, if_name - src);
    src_buf[if_name - src] = 0;
    src_ptr = src_buf;
  }

  int result = error_wrapper(::inet_pton(af, src_ptr, dest), ec);
  if (result <= 0 && !ec)
    ec = asio::error::invalid_argument;
  if (result > 0 && is_v6 && scope_id)
  {
    using namespace std; // For strchr and atoi.
    *scope_id = 0;
    if (if_name != 0)
    {
      in6_addr_type* ipv6_address = static_cast<in6_addr_type*>(dest);
      bool is_link_local = ((ipv6_address->s6_addr[0] == 0xfe)
          && ((ipv6_address->s6_addr[1] & 0xc0) == 0x80));
      bool is_multicast_link_local = ((ipv6_address->s6_addr[0] == 0xff)
          && ((ipv6_address->s6_addr[1] & 0x0f) == 0x02));
      if (is_link_local || is_multicast_link_local)
        *scope_id = if_nametoindex(if_name + 1);
      if (*scope_id == 0)
        *scope_id = atoi(if_name + 1);
    }
  }
  return result;
}

int gethostname(char* name, int namelen, asio::error_code& ec)
{
  clear_last_error();

  int result = error_wrapper(::gethostname(name, namelen), ec);

  return result;
}

#if !defined(ASIO_WINDOWS_RUNTIME)

#if !defined(ASIO_HAS_GETADDRINFO)

// The following functions are only needed for emulation of getaddrinfo and
// getnameinfo.

inline asio::error_code translate_netdb_error(int error)
{
  switch (error)
  {
  case 0:
    return asio::error_code();
  case HOST_NOT_FOUND:
    return asio::error::host_not_found;
  case TRY_AGAIN:
    return asio::error::host_not_found_try_again;
  case NO_RECOVERY:
    return asio::error::no_recovery;
  case NO_DATA:
    return asio::error::no_data;
  default:
    ASIO_ASSERT(false);
    return asio::error::invalid_argument;
  }
}

inline hostent* gethostbyaddr(const char* addr, int length, int af,
    hostent* result, char* buffer, int buflength, asio::error_code& ec)
{
  clear_last_error();

  hostent* retval = 0;
  int error = 0;
  error_wrapper(::gethostbyaddr_r(addr, length, af, result, buffer,
        buflength, &retval, &error), ec);
  if (error)
    ec = translate_netdb_error(error);
  return retval;
}

inline hostent* gethostbyname(const char* name, int af, struct hostent* result,
    char* buffer, int buflength, int ai_flags, asio::error_code& ec)
{
  clear_last_error();

  (void)(ai_flags);
  if (af != ASIO_OS_DEF(AF_INET))
  {
    ec = asio::error::address_family_not_supported;
    return 0;
  }
  hostent* retval = 0;
  int error = 0;
  error_wrapper(::gethostbyname_r(name, result,
        buffer, buflength, &retval, &error), ec);
  if (error)
    ec = translate_netdb_error(error);
  return retval;
}

inline void freehostent(hostent* h)
{
  (void)(h);
}

// Emulation of getaddrinfo based on implementation in:
// Stevens, W. R., UNIX Network Programming Vol. 1, 2nd Ed., Prentice-Hall 1998.

struct gai_search
{
  const char* host;
  int family;
};

inline int gai_nsearch(const char* host,
    const addrinfo_type* hints, gai_search (&search)[2])
{
  int search_count = 0;
  if (host == 0 || host[0] == '\0')
  {
    if (hints->ai_flags & AI_PASSIVE)
    {
      // No host and AI_PASSIVE implies wildcard bind.
      switch (hints->ai_family)
      {
      case ASIO_OS_DEF(AF_INET):
        search[search_count].host = "0.0.0.0";
        search[search_count].family = ASIO_OS_DEF(AF_INET);
        ++search_count;
        break;
      case ASIO_OS_DEF(AF_INET6):
        search[search_count].host = "0::0";
        search[search_count].family = ASIO_OS_DEF(AF_INET6);
        ++search_count;
        break;
      case ASIO_OS_DEF(AF_UNSPEC):
        search[search_count].host = "0::0";
        search[search_count].family = ASIO_OS_DEF(AF_INET6);
        ++search_count;
        search[search_count].host = "0.0.0.0";
        search[search_count].family = ASIO_OS_DEF(AF_INET);
        ++search_count;
        break;
      default:
        break;
      }
    }
    else
    {
      // No host and not AI_PASSIVE means connect to local host.
      switch (hints->ai_family)
      {
      case ASIO_OS_DEF(AF_INET):
        search[search_count].host = "localhost";
        search[search_count].family = ASIO_OS_DEF(AF_INET);
        ++search_count;
        break;
      case ASIO_OS_DEF(AF_INET6):
        search[search_count].host = "localhost";
        search[search_count].family = ASIO_OS_DEF(AF_INET6);
        ++search_count;
        break;
      case ASIO_OS_DEF(AF_UNSPEC):
        search[search_count].host = "localhost";
        search[search_count].family = ASIO_OS_DEF(AF_INET6);
        ++search_count;
        search[search_count].host = "localhost";
        search[search_count].family = ASIO_OS_DEF(AF_INET);
        ++search_count;
        break;
      default:
        break;
      }
    }
  }
  else
  {
    // Host is specified.
    switch (hints->ai_family)
    {
    case ASIO_OS_DEF(AF_INET):
      search[search_count].host = host;
      search[search_count].family = ASIO_OS_DEF(AF_INET);
      ++search_count;
      break;
    case ASIO_OS_DEF(AF_INET6):
      search[search_count].host = host;
      search[search_count].family = ASIO_OS_DEF(AF_INET6);
      ++search_count;
      break;
    case ASIO_OS_DEF(AF_UNSPEC):
      search[search_count].host = host;
      search[search_count].family = ASIO_OS_DEF(AF_INET6);
      ++search_count;
      search[search_count].host = host;
      search[search_count].family = ASIO_OS_DEF(AF_INET);
      ++search_count;
      break;
    default:
      break;
    }
  }
  return search_count;
}

template <typename T>
inline T* gai_alloc(std::size_t size = sizeof(T))
{
  using namespace std;
  T* p = static_cast<T*>(::operator new(size, std::nothrow));
  if (p)
    memset(p, 0, size);
  return p;
}

inline void gai_free(void* p)
{
  ::operator delete(p);
}

inline void gai_strcpy(char* target, const char* source, std::size_t max_size)
{
  using namespace std;
#if defined(ASIO_HAS_SECURE_RTL)
  strcpy_s(target, max_size, source);
#else // defined(ASIO_HAS_SECURE_RTL)
  *target = 0;
  if (max_size > 0)
    strncat(target, source, max_size - 1);
#endif // defined(ASIO_HAS_SECURE_RTL)
}

enum { gai_clone_flag = 1 << 30 };

inline int gai_aistruct(addrinfo_type*** next, const addrinfo_type* hints,
    const void* addr, int family)
{
  using namespace std;

  addrinfo_type* ai = gai_alloc<addrinfo_type>();
  if (ai == 0)
    return EAI_MEMORY;

  ai->ai_next = 0;
  **next = ai;
  *next = &ai->ai_next;

  ai->ai_canonname = 0;
  ai->ai_socktype = hints->ai_socktype;
  if (ai->ai_socktype == 0)
    ai->ai_flags |= gai_clone_flag;
  ai->ai_protocol = hints->ai_protocol;
  ai->ai_family = family;

  switch (ai->ai_family)
  {
  case ASIO_OS_DEF(AF_INET):
    {
      sockaddr_in4_type* sinptr = gai_alloc<sockaddr_in4_type>();
      if (sinptr == 0)
        return EAI_MEMORY;
      sinptr->sin_family = ASIO_OS_DEF(AF_INET);
      memcpy(&sinptr->sin_addr, addr, sizeof(in4_addr_type));
      ai->ai_addr = reinterpret_cast<sockaddr*>(sinptr);
      ai->ai_addrlen = sizeof(sockaddr_in4_type);
      break;
    }
  case ASIO_OS_DEF(AF_INET6):
    {
      sockaddr_in6_type* sin6ptr = gai_alloc<sockaddr_in6_type>();
      if (sin6ptr == 0)
        return EAI_MEMORY;
      sin6ptr->sin6_family = ASIO_OS_DEF(AF_INET6);
      memcpy(&sin6ptr->sin6_addr, addr, sizeof(in6_addr_type));
      ai->ai_addr = reinterpret_cast<sockaddr*>(sin6ptr);
      ai->ai_addrlen = sizeof(sockaddr_in6_type);
      break;
    }
  default:
    break;
  }

  return 0;
}

inline addrinfo_type* gai_clone(addrinfo_type* ai)
{
  using namespace std;

  addrinfo_type* new_ai = gai_alloc<addrinfo_type>();
  if (new_ai == 0)
    return new_ai;

  new_ai->ai_next = ai->ai_next;
  ai->ai_next = new_ai;

  new_ai->ai_flags = 0;
  new_ai->ai_family = ai->ai_family;
  new_ai->ai_socktype = ai->ai_socktype;
  new_ai->ai_protocol = ai->ai_protocol;
  new_ai->ai_canonname = 0;
  new_ai->ai_addrlen = ai->ai_addrlen;
  new_ai->ai_addr = gai_alloc<sockaddr>(ai->ai_addrlen);
  memcpy(new_ai->ai_addr, ai->ai_addr, ai->ai_addrlen);

  return new_ai;
}

inline int gai_port(addrinfo_type* aihead, int port, int socktype)
{
  int num_found = 0;

  for (addrinfo_type* ai = aihead; ai; ai = ai->ai_next)
  {
    if (ai->ai_flags & gai_clone_flag)
    {
      if (ai->ai_socktype != 0)
      {
        ai = gai_clone(ai);
        if (ai == 0)
          return -1;
        // ai now points to newly cloned entry.
      }
    }
    else if (ai->ai_socktype != socktype)
    {
      // Ignore if mismatch on socket type.
      continue;
    }

    ai->ai_socktype = socktype;

    switch (ai->ai_family)
    {
    case ASIO_OS_DEF(AF_INET):
      {
        sockaddr_in4_type* sinptr =
          reinterpret_cast<sockaddr_in4_type*>(ai->ai_addr);
        sinptr->sin_port = port;
        ++num_found;
        break;
      }
    case ASIO_OS_DEF(AF_INET6):
      {
        sockaddr_in6_type* sin6ptr =
          reinterpret_cast<sockaddr_in6_type*>(ai->ai_addr);
        sin6ptr->sin6_port = port;
        ++num_found;
        break;
      }
    default:
      break;
    }
  }

  return num_found;
}

inline int gai_serv(addrinfo_type* aihead,
    const addrinfo_type* hints, const char* serv)
{
  using namespace std;

  int num_found = 0;

  if (
#if defined(AI_NUMERICSERV)
      (hints->ai_flags & AI_NUMERICSERV) ||
#endif
      isdigit(static_cast<unsigned char>(serv[0])))
  {
    int port = htons(atoi(serv));
    if (hints->ai_socktype)
    {
      // Caller specifies socket type.
      int rc = gai_port(aihead, port, hints->ai_socktype);
      if (rc < 0)
        return EAI_MEMORY;
      num_found += rc;
    }
    else
    {
      // Caller does not specify socket type.
      int rc = gai_port(aihead, port, SOCK_STREAM);
      if (rc < 0)
        return EAI_MEMORY;
      num_found += rc;
      rc = gai_port(aihead, port, SOCK_DGRAM);
      if (rc < 0)
        return EAI_MEMORY;
      num_found += rc;
    }
  }
  else
  {
    // Try service name with TCP first, then UDP.
    if (hints->ai_socktype == 0 || hints->ai_socktype == SOCK_STREAM)
    {
      servent* sptr = getservbyname(serv, "tcp");
      if (sptr != 0)
      {
        int rc = gai_port(aihead, sptr->s_port, SOCK_STREAM);
        if (rc < 0)
          return EAI_MEMORY;
        num_found += rc;
      }
    }
    if (hints->ai_socktype == 0 || hints->ai_socktype == SOCK_DGRAM)
    {
      servent* sptr = getservbyname(serv, "udp");
      if (sptr != 0)
      {
        int rc = gai_port(aihead, sptr->s_port, SOCK_DGRAM);
        if (rc < 0)
          return EAI_MEMORY;
        num_found += rc;
      }
    }
  }

  if (num_found == 0)
  {
    if (hints->ai_socktype == 0)
    {
      // All calls to getservbyname() failed.
      return EAI_NONAME;
    }
    else
    {
      // Service not supported for socket type.
      return EAI_SERVICE;
    }
  }

  return 0;
}

inline int gai_echeck(const char* host, const char* service,
    int flags, int family, int socktype, int protocol)
{
  (void)(flags);
  (void)(protocol);

  // Host or service must be specified.
  if (host == 0 || host[0] == '\0')
    if (service == 0 || service[0] == '\0')
      return EAI_NONAME;

  // Check combination of family and socket type.
  switch (family)
  {
  case ASIO_OS_DEF(AF_UNSPEC):
    break;
  case ASIO_OS_DEF(AF_INET):
  case ASIO_OS_DEF(AF_INET6):
    if (service != 0 && service[0] != '\0')
      if (socktype != 0 && socktype != SOCK_STREAM && socktype != SOCK_DGRAM)
        return EAI_SOCKTYPE;
    break;
  default:
    return EAI_FAMILY;
  }

  return 0;
}

inline void freeaddrinfo_emulation(addrinfo_type* aihead)
{
  addrinfo_type* ai = aihead;
  while (ai)
  {
    gai_free(ai->ai_addr);
    gai_free(ai->ai_canonname);
    addrinfo_type* ainext = ai->ai_next;
    gai_free(ai);
    ai = ainext;
  }
}

inline int getaddrinfo_emulation(const char* host, const char* service,
    const addrinfo_type* hintsp, addrinfo_type** result)
{
  // Set up linked list of addrinfo structures.
  addrinfo_type* aihead = 0;
  addrinfo_type** ainext = &aihead;
  char* canon = 0;

  // Supply default hints if not specified by caller.
  addrinfo_type hints = addrinfo_type();
  hints.ai_family = ASIO_OS_DEF(AF_UNSPEC);
  if (hintsp)
    hints = *hintsp;

  // If the resolution is not specifically for AF_INET6, remove the AI_V4MAPPED
  // and AI_ALL flags.
#if defined(AI_V4MAPPED)
  if (hints.ai_family != ASIO_OS_DEF(AF_INET6))
    hints.ai_flags &= ~AI_V4MAPPED;
#endif
#if defined(AI_ALL)
  if (hints.ai_family != ASIO_OS_DEF(AF_INET6))
    hints.ai_flags &= ~AI_ALL;
#endif

  // Basic error checking.
  int rc = gai_echeck(host, service, hints.ai_flags, hints.ai_family,
      hints.ai_socktype, hints.ai_protocol);
  if (rc != 0)
  {
    freeaddrinfo_emulation(aihead);
    return rc;
  }

  gai_search search[2];
  int search_count = gai_nsearch(host, &hints, search);
  for (gai_search* sptr = search; sptr < search + search_count; ++sptr)
  {
    // Check for IPv4 dotted decimal string.
    in4_addr_type inaddr;
    asio::error_code ec;
    if (socket_ops::inet_pton(ASIO_OS_DEF(AF_INET),
          sptr->host, &inaddr, 0, ec) == 1)
    {
      if (hints.ai_family != ASIO_OS_DEF(AF_UNSPEC)
          && hints.ai_family != ASIO_OS_DEF(AF_INET))
      {
        freeaddrinfo_emulation(aihead);
        gai_free(canon);
        return EAI_FAMILY;
      }
      if (sptr->family == ASIO_OS_DEF(AF_INET))
      {
        rc = gai_aistruct(&ainext, &hints, &inaddr, ASIO_OS_DEF(AF_INET));
        if (rc != 0)
        {
          freeaddrinfo_emulation(aihead);
          gai_free(canon);
          return rc;
        }
      }
      continue;
    }

    // Check for IPv6 hex string.
    in6_addr_type in6addr;
    if (socket_ops::inet_pton(ASIO_OS_DEF(AF_INET6),
          sptr->host, &in6addr, 0, ec) == 1)
    {
      if (hints.ai_family != ASIO_OS_DEF(AF_UNSPEC)
          && hints.ai_family != ASIO_OS_DEF(AF_INET6))
      {
        freeaddrinfo_emulation(aihead);
        gai_free(canon);
        return EAI_FAMILY;
      }
      if (sptr->family == ASIO_OS_DEF(AF_INET6))
      {
        rc = gai_aistruct(&ainext, &hints, &in6addr,
            ASIO_OS_DEF(AF_INET6));
        if (rc != 0)
        {
          freeaddrinfo_emulation(aihead);
          gai_free(canon);
          return rc;
        }
      }
      continue;
    }

    // Look up hostname.
    hostent hent;
    char hbuf[8192] = "";
    hostent* hptr = socket_ops::gethostbyname(sptr->host,
        sptr->family, &hent, hbuf, sizeof(hbuf), hints.ai_flags, ec);
    if (hptr == 0)
    {
      if (search_count == 2)
      {
        // Failure is OK if there are multiple searches.
        continue;
      }
      freeaddrinfo_emulation(aihead);
      gai_free(canon);
      if (ec == asio::error::host_not_found)
        return EAI_NONAME;
      if (ec == asio::error::host_not_found_try_again)
        return EAI_AGAIN;
      if (ec == asio::error::no_recovery)
        return EAI_FAIL;
      if (ec == asio::error::no_data)
        return EAI_NONAME;
      return EAI_NONAME;
    }

    // Check for address family mismatch if one was specified.
    if (hints.ai_family != ASIO_OS_DEF(AF_UNSPEC)
        && hints.ai_family != hptr->h_addrtype)
    {
      freeaddrinfo_emulation(aihead);
      gai_free(canon);
      socket_ops::freehostent(hptr);
      return EAI_FAMILY;
    }

    // Save canonical name first time.
    if (host != 0 && host[0] != '\0' && hptr->h_name && hptr->h_name[0]
        && (hints.ai_flags & AI_CANONNAME) && canon == 0)
    {
      std::size_t canon_len = strlen(hptr->h_name) + 1;
      canon = gai_alloc<char>(canon_len);
      if (canon == 0)
      {
        freeaddrinfo_emulation(aihead);
        socket_ops::freehostent(hptr);
        return EAI_MEMORY;
      }
      gai_strcpy(canon, hptr->h_name, canon_len);
    }

    // Create an addrinfo structure for each returned address.
    for (char** ap = hptr->h_addr_list; *ap; ++ap)
    {
      rc = gai_aistruct(&ainext, &hints, *ap, hptr->h_addrtype);
      if (rc != 0)
      {
        freeaddrinfo_emulation(aihead);
        gai_free(canon);
        socket_ops::freehostent(hptr);
        return EAI_FAMILY;
      }
    }

    socket_ops::freehostent(hptr);
  }

  // Check if we found anything.
  if (aihead == 0)
  {
    gai_free(canon);
    return EAI_NONAME;
  }

  // Return canonical name in first entry.
  if (host != 0 && host[0] != '\0' && (hints.ai_flags & AI_CANONNAME))
  {
    if (canon)
    {
      aihead->ai_canonname = canon;
      canon = 0;
    }
    else
    {
      std::size_t canonname_len = strlen(search[0].host) + 1;
      aihead->ai_canonname = gai_alloc<char>(canonname_len);
      if (aihead->ai_canonname == 0)
      {
        freeaddrinfo_emulation(aihead);
        return EAI_MEMORY;
      }
      gai_strcpy(aihead->ai_canonname, search[0].host, canonname_len);
    }
  }
  gai_free(canon);

  // Process the service name.
  if (service != 0 && service[0] != '\0')
  {
    rc = gai_serv(aihead, &hints, service);
    if (rc != 0)
    {
      freeaddrinfo_emulation(aihead);
      return rc;
    }
  }

  // Return result to caller.
  *result = aihead;
  return 0;
}

inline asio::error_code getnameinfo_emulation(
    const socket_addr_type* sa, std::size_t salen, char* host,
    std::size_t hostlen, char* serv, std::size_t servlen, int flags,
    asio::error_code& ec)
{
  using namespace std;

  const char* addr;
  size_t addr_len;
  unsigned short port;
  switch (sa->sa_family)
  {
  case ASIO_OS_DEF(AF_INET):
    if (salen != sizeof(sockaddr_in4_type))
    {
      return ec = asio::error::invalid_argument;
    }
    addr = reinterpret_cast<const char*>(
        &reinterpret_cast<const sockaddr_in4_type*>(sa)->sin_addr);
    addr_len = sizeof(in4_addr_type);
    port = reinterpret_cast<const sockaddr_in4_type*>(sa)->sin_port;
    break;
  case ASIO_OS_DEF(AF_INET6):
    if (salen != sizeof(sockaddr_in6_type))
    {
      return ec = asio::error::invalid_argument;
    }
    addr = reinterpret_cast<const char*>(
        &reinterpret_cast<const sockaddr_in6_type*>(sa)->sin6_addr);
    addr_len = sizeof(in6_addr_type);
    port = reinterpret_cast<const sockaddr_in6_type*>(sa)->sin6_port;
    break;
  default:
    return ec = asio::error::address_family_not_supported;
  }

  if (host && hostlen > 0)
  {
    if (flags & NI_NUMERICHOST)
    {
      if (socket_ops::inet_ntop(sa->sa_family, addr, host, hostlen, 0, ec) == 0)
      {
        return ec;
      }
    }
    else
    {
      hostent hent;
      char hbuf[8192] = "";
      hostent* hptr = socket_ops::gethostbyaddr(addr,
          static_cast<int>(addr_len), sa->sa_family,
          &hent, hbuf, sizeof(hbuf), ec);
      if (hptr && hptr->h_name && hptr->h_name[0] != '\0')
      {
        if (flags & NI_NOFQDN)
        {
          char* dot = strchr(hptr->h_name, '.');
          if (dot)
          {
            *dot = 0;
          }
        }
        gai_strcpy(host, hptr->h_name, hostlen);
        socket_ops::freehostent(hptr);
      }
      else
      {
        socket_ops::freehostent(hptr);
        if (flags & NI_NAMEREQD)
        {
          return ec = asio::error::host_not_found;
        }
        if (socket_ops::inet_ntop(sa->sa_family,
              addr, host, hostlen, 0, ec) == 0)
        {
          return ec;
        }
      }
    }
  }

  if (serv && servlen > 0)
  {
    if (flags & NI_NUMERICSERV)
    {
      if (servlen < 6)
      {
        return ec = asio::error::no_buffer_space;
      }
#if defined(ASIO_HAS_SECURE_RTL)
      sprintf_s(serv, servlen, "%u", ntohs(port));
#else // defined(ASIO_HAS_SECURE_RTL)
      sprintf(serv, "%u", ntohs(port));
#endif // defined(ASIO_HAS_SECURE_RTL)
    }
    else
    {
#if defined(ASIO_HAS_PTHREADS)
      static ::pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
      ::pthread_mutex_lock(&mutex);
#endif // defined(ASIO_HAS_PTHREADS)
      servent* sptr = ::getservbyport(port, (flags & NI_DGRAM) ? "udp" : 0);
      if (sptr && sptr->s_name && sptr->s_name[0] != '\0')
      {
        gai_strcpy(serv, sptr->s_name, servlen);
      }
      else
      {
        if (servlen < 6)
        {
          return ec = asio::error::no_buffer_space;
        }
#if defined(ASIO_HAS_SECURE_RTL)
        sprintf_s(serv, servlen, "%u", ntohs(port));
#else // defined(ASIO_HAS_SECURE_RTL)
        sprintf(serv, "%u", ntohs(port));
#endif // defined(ASIO_HAS_SECURE_RTL)
      }
#if defined(ASIO_HAS_PTHREADS)
      ::pthread_mutex_unlock(&mutex);
#endif // defined(ASIO_HAS_PTHREADS)
    }
  }

  ec = asio::error_code();
  return ec;
}

#endif // !defined(ASIO_HAS_GETADDRINFO)

inline asio::error_code translate_addrinfo_error(int error)
{
  switch (error)
  {
  case 0:
    return asio::error_code();
  case EAI_AGAIN:
    return asio::error::host_not_found_try_again;
  case EAI_BADFLAGS:
    return asio::error::invalid_argument;
  case EAI_FAIL:
    return asio::error::no_recovery;
  case EAI_FAMILY:
    return asio::error::address_family_not_supported;
  case EAI_MEMORY:
    return asio::error::no_memory;
  case EAI_NONAME:
#if defined(EAI_ADDRFAMILY)
  case EAI_ADDRFAMILY:
#endif
#if defined(EAI_NODATA) && (EAI_NODATA != EAI_NONAME)
  case EAI_NODATA:
#endif
    return asio::error::host_not_found;
  case EAI_SERVICE:
    return asio::error::service_not_found;
  case EAI_SOCKTYPE:
    return asio::error::socket_type_not_supported;
  default: // Possibly the non-portable EAI_SYSTEM.
    return asio::error_code(
        errno, asio::error::get_system_category());
  }
}

asio::error_code getaddrinfo(const char* host,
    const char* service, const addrinfo_type& hints,
    addrinfo_type** result, asio::error_code& ec)
{
  host = (host && *host) ? host : 0;
  service = (service && *service) ? service : 0;
  clear_last_error();

#if !defined(ASIO_HAS_GETADDRINFO)
  int error = getaddrinfo_emulation(host, service, &hints, result);
  return ec = translate_addrinfo_error(error);
#else
  int error = ::getaddrinfo(host, service, &hints, result);
  return ec = translate_addrinfo_error(error);
#endif
}

asio::error_code background_getaddrinfo(
    const weak_cancel_token_type& cancel_token, const char* host,
    const char* service, const addrinfo_type& hints,
    addrinfo_type** result, asio::error_code& ec)
{
  if (cancel_token.expired())
    ec = asio::error::operation_aborted;
  else
    socket_ops::getaddrinfo(host, service, hints, result, ec);
  return ec;
}

void freeaddrinfo(addrinfo_type* ai)
{
#if !defined(ASIO_HAS_GETADDRINFO)
  freeaddrinfo_emulation(ai);
#else
  ::freeaddrinfo(ai);
#endif
}

asio::error_code getnameinfo(const socket_addr_type* addr,
    std::size_t addrlen, char* host, std::size_t hostlen,
    char* serv, std::size_t servlen, int flags, asio::error_code& ec)
{
#if !defined(ASIO_HAS_GETADDRINFO)
  using namespace std; // For memcpy.
  sockaddr_storage_type tmp_addr;
  memcpy(&tmp_addr, addr, addrlen);
  addr = reinterpret_cast<socket_addr_type*>(&tmp_addr);
  clear_last_error();
  return getnameinfo_emulation(addr, addrlen,
      host, hostlen, serv, servlen, flags, ec);
#else
  clear_last_error();
  int error = ::getnameinfo(addr, addrlen, host, hostlen, serv, servlen, flags);
  return ec = translate_addrinfo_error(error);
#endif
}

asio::error_code sync_getnameinfo(
    const socket_addr_type* addr, std::size_t addrlen,
    char* host, std::size_t hostlen, char* serv,
    std::size_t servlen, int sock_type, asio::error_code& ec)
{
  // First try resolving with the service name. If that fails try resolving
  // but allow the service to be returned as a number.
  int flags = (sock_type == SOCK_DGRAM) ? NI_DGRAM : 0;
  socket_ops::getnameinfo(addr, addrlen, host,
      hostlen, serv, servlen, flags, ec);
  if (ec)
  {
    socket_ops::getnameinfo(addr, addrlen, host, hostlen,
        serv, servlen, flags | NI_NUMERICSERV, ec);
  }

  return ec;
}

asio::error_code background_getnameinfo(
    const weak_cancel_token_type& cancel_token,
    const socket_addr_type* addr, std::size_t addrlen,
    char* host, std::size_t hostlen, char* serv,
    std::size_t servlen, int sock_type, asio::error_code& ec)
{
  if (cancel_token.expired())
  {
    ec = asio::error::operation_aborted;
  }
  else
  {
    // First try resolving with the service name. If that fails try resolving
    // but allow the service to be returned as a number.
    int flags = (sock_type == SOCK_DGRAM) ? NI_DGRAM : 0;
    socket_ops::getnameinfo(addr, addrlen, host,
        hostlen, serv, servlen, flags, ec);
    if (ec)
    {
      socket_ops::getnameinfo(addr, addrlen, host, hostlen,
          serv, servlen, flags | NI_NUMERICSERV, ec);
    }
  }

  return ec;
}

#endif // !defined(ASIO_WINDOWS_RUNTIME)

u_long_type network_to_host_long(u_long_type value)
{
  return ntohl(value);
}

u_long_type host_to_network_long(u_long_type value)
{
  return htonl(value);
}

u_short_type network_to_host_short(u_short_type value)
{
  return ntohs(value);
}

u_short_type host_to_network_short(u_short_type value)
{
  return htons(value);
}

} // namespace socket_ops
} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_SOCKET_OPS_IPP
