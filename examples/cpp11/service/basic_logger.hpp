#ifndef SERVICES_BASIC_LOGGER_HPP
#define SERVICES_BASIC_LOGGER_HPP

#include <asio.hpp>
#include <string>

namespace services {

template <typename Service>
class basic_logger
  : private asio::detail::noncopyable
{
public:
  typedef Service service_type;
  typedef typename service_type::impl_type impl_type;

  explicit basic_logger(asio::io_context& io_context,
                        const std::string& identifier)
    : service_(asio::use_service<Service>(io_context)),
      impl_(service_.create(identifier))
  {
  }

  ~basic_logger()
  {
    service_.destroy(impl_);
  }

  void use_file(const std::string& file)
  {
    service_.use_file(impl_, file);
  }

  void log(const std::string& message)
  {
    service_.log(impl_, message);
  }

private:
  service_type& service_;
  impl_type impl_;
};

} // namespace services

#endif // SERVICES_BASIC_LOGGER_HPP
