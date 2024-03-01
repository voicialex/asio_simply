//
// detail/pop_options.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// No header guard

#if defined(__clang__)

// Clang

# if defined(__OBJC__)
#  if !defined(__APPLE_CC__) || (__APPLE_CC__ <= 1)
#   if defined(ASIO_OBJC_WORKAROUND)
#    undef Protocol
#    undef id
#    undef ASIO_OBJC_WORKAROUND
#   endif
#  endif
# endif

# if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32)
#  pragma GCC visibility pop
# endif // !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32)

#elif defined(__GNUC__)

// GNU C++

# if defined(__MINGW32__) || defined(__CYGWIN__)
#  pragma pack (pop)
# endif

# if defined(__OBJC__)
#  if !defined(__APPLE_CC__) || (__APPLE_CC__ <= 1)
#   if defined(ASIO_OBJC_WORKAROUND)
#    undef Protocol
#    undef id
#    undef ASIO_OBJC_WORKAROUND
#   endif
#  endif
# endif

# if (__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || (__GNUC__ > 4)
#  pragma GCC visibility pop
# endif // (__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || (__GNUC__ > 4)

# if (__GNUC__ >= 7)
#  pragma GCC diagnostic pop
# endif // (__GNUC__ >= 7)


#endif
