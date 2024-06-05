#ifndef ASIO_DETAIL_REGEX_FWD_HPP
#define ASIO_DETAIL_REGEX_FWD_HPP

#if defined(ASIO_HAS_BOOST_REGEX)

// #include <boost/regex_fwd.hpp>
// #include <boost/regex/v4/match_flags.hpp>

namespace boost {

template <class BidiIterator>
struct sub_match;

template <class BidiIterator, class Allocator>
class match_results;

} // namespace boost

#endif // defined(ASIO_HAS_BOOST_REGEX)

#endif // ASIO_DETAIL_REGEX_FWD_HPP
