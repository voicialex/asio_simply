#ifndef SERVICES_LOGGER_SERVICE_HPP
#define SERVICES_LOGGER_SERVICE_HPP

#include <asio.hpp>
#include <functional>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
namespace services
{
class logger_service
    : public asio::io_context::service
{
public:
    typedef logger_service key_type;
    static asio::io_context::id id;

    struct logger_impl
    {
        explicit logger_impl(const std::string &ident) : identifier(ident) {}
        std::string identifier;
        std::ofstream ofstream; // Each logger_impl manages its own ofstream
    };

    typedef logger_impl *impl_type;

    explicit logger_service(asio::io_context &io_context)
        : asio::io_context::service(io_context),
          work_io_context_(io_context),
          work_(asio::make_work_guard(io_context))
    {
        // Start the work thread
        work_thread_ = std::thread([this] {
            work_io_context_.run();
        });
    }

    void shutdown_service()
    {
        work_io_context_.stop();
        if (work_thread_.joinable())
            work_thread_.join();
    }

    ~logger_service()
    {
        shutdown_service();
    }

    impl_type create(const std::string &identifier)
    {
        return new logger_impl(identifier);
    }

    void destroy(impl_type impl)
    {
        delete impl;
    }

    void use_file(impl_type impl, const std::string &file)
    {
        impl->ofstream.close();
        impl->ofstream.clear();
        impl->ofstream.open(file.c_str());
    }

    void log(impl_type impl, const std::string &message)
    {
        asio::post(work_io_context_, [this, impl, message] {
            log_impl(impl, message);
        });
    }

private:
    void log_impl(impl_type impl, const std::string &message)
    {
        if (impl->ofstream.is_open())
        {
            impl->ofstream << impl->identifier << ": " << message << std::endl;
        }
        else
        {
            std::cerr << "Error: Log file is not open for logger " << impl->identifier << std::endl;
        }
    }

    asio::io_context &work_io_context_;
    asio::executor_work_guard<asio::io_context::executor_type> work_;
    std::thread work_thread_;
};

asio::io_context::id logger_service::id;

} // namespace services

#endif // SERVICES_LOGGER_SERVICE_HPP
