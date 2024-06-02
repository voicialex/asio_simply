#ifndef ASIO_DETAIL_ARRAY_FWD_HPP
#define ASIO_DETAIL_ARRAY_FWD_HPP

#include "asio/detail/config.hpp"

namespace boost {

template<class T, std::size_t N>
class array;

} // namespace boost

// Standard library components can't be forward declared, so we'll have to
// include the array header. Fortunately, it's fairly lightweight and doesn't
// add significantly to the compile time.
#if defined(ASIO_HAS_STD_ARRAY)
# include <array>
#endif // defined(ASIO_HAS_STD_ARRAY)

#endif // ASIO_DETAIL_ARRAY_FWD_HPP
