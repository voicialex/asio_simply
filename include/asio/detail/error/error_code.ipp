#ifndef ASIO_IMPL_ERROR_CODE_IPP
#define ASIO_IMPL_ERROR_CODE_IPP

#include "asio/detail/config.hpp"

# include <cerrno>
# include <cstring>
# include <string>

// #include "asio/detail/local_free_on_block_exit.hpp"
#include "asio/detail/socket_types.hpp"
#include "asio/detail/error/error_code.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

class system_category : public error_category
{
public:
  const char* name() const ASIO_ERROR_CATEGORY_NOEXCEPT
  {
    return "asio.system";
  }

  std::string message(int value) const
  {
    char buf[256] = "";
    using namespace std;
    return strerror_result(strerror_r(value, buf, sizeof(buf)), buf);
  }

private:
  // Helper function to adapt the result from glibc's variant of strerror_r.
  static const char* strerror_result(int, const char* s) { return s; }
  static const char* strerror_result(const char* s, const char*) { return s; }
};

} // namespace detail

const error_category& system_category()
{
  static detail::system_category instance;
  return instance;
}

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IMPL_ERROR_CODE_IPP
