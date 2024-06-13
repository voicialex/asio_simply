
#include <iostream>
#include <string>
#include <thread>
#ifdef __linux__
#include <pthread.h>
#endif
#include "asio.hpp"
#include "log_message.hpp"
#include <deque>
#include <functional>
#include <memory>

#define ENABLE_HEARTBEAT

typedef std::function<int(void *, const char *, int)> OnRecvCallback;

class LogClientImpl;
class LogClient
{
public:
    LogClient(const std::string &host, const std::string &port);

    ~LogClient();

    void start();

    void write(const std::string &msg);

    void set_callback(OnRecvCallback func = OnRecvCallback());

private:
    LogClientImpl *pimpl_;
};

///////////////////////////////////////////

using asio::steady_timer;
using asio::ip::tcp;

class LogClientImpl
{
public:
    LogClientImpl(const std::string &host, const std::string &port);

    ~LogClientImpl();

    void async_write(const std::string &msg);

    void set_callback(OnRecvCallback func = OnRecvCallback());

    void start();

    void close();

private:
    void start_connect();

    void retry_connect(const char *err);

    void on_connect();

    void do_async_read();

    void do_async_write();

    void do_heartbeat();

private:
    asio::io_context io_context_;
    tcp::socket socket_;
    tcp::resolver::results_type endpoints_;
    steady_timer heartbeat_timer_;
    std::string read_msg_;
    std::deque<std::string> write_msgs_;
    OnRecvCallback on_recv_;
    std::thread io_thread_;
    bool is_connected_;
};

LogClientImpl::LogClientImpl(const std::string &host, const std::string &port)
    : socket_(io_context_), heartbeat_timer_(io_context_), is_connected_(false)
{
    // 在构造函数内部解析主机和端口
    tcp::resolver resolver(io_context_);
    endpoints_ = resolver.resolve(host, port);
}

LogClientImpl::~LogClientImpl()
{
    is_connected_ = false;
    heartbeat_timer_.cancel();
    close();
    io_context_.stop();
    if (io_thread_.joinable())
    {
        io_thread_.join();
    }
}

void LogClientImpl::start()
{
    start_connect();

    io_thread_ = std::thread([this]()
                             {
#ifdef __linux__
        pthread_setname_np(pthread_self(), "IOThread.Cli");
#endif
        THROW_C3LOG_VERBOSE("io_context_ BEGIN ...");
        io_context_.run();
        THROW_C3LOG_VERBOSE("io_context_ DONE ..."); });
}

void LogClientImpl::async_write(const std::string &msg)
{
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress)
    {
        do_async_write();
    }
}

void LogClientImpl::do_async_write()
{
    asio::async_write(socket_, asio::buffer(write_msgs_.front()),
                      [this](std::error_code ec, std::size_t len)
                      {
                          if (!ec)
                          {
                              write_msgs_.pop_front();
                              if (!write_msgs_.empty())
                              {
                                  do_async_write();
                              }
                          }
                          else
                          {
                              THROW_C3LOG_EXCEPTION("Error in async_write: %s", ec.message().c_str());
                              socket_.close();
                          }
                      });
}

void LogClientImpl::set_callback(OnRecvCallback func)
{
    if (func)
    {
        on_recv_ = func;
    }
}

void LogClientImpl::close()
{
    asio::post(io_context_, [this]()
               { socket_.close(); });
}

void LogClientImpl::start_connect()
{
    THROW_C3LOG_VERBOSE("start_connect ...");
    asio::async_connect(socket_, endpoints_,
                        [this](std::error_code ec, tcp::endpoint endpoint)
                        {
                            if (!ec)
                            {
                                on_connect();
                            }
                            else
                            {
                                retry_connect("start_connect");
                            }
                        });
}

void LogClientImpl::on_connect()
{
    auto msg = std::make_shared<log_msg>();
    msg->set_online_info();

    // Capture by value to ensure the shared_ptr is kept alive until the async operation completes
    asio::async_write(socket_, asio::buffer(msg->to_string()), [this, msg](std::error_code ec, std::size_t len)
                      {
        if (!ec)
        {
            THROW_C3LOG_VERBOSE("on_connect ...");
            // send success
            is_connected_ = true;
            do_async_read();
            do_heartbeat();
        }
        else if (ec == asio::error::eof)
        {
            retry_connect("async_write");
        }
        else
        {
            socket_.close();
        } });
}

void LogClientImpl::retry_connect(const char *err)
{
    THROW_C3LOG_VERBOSE("err: %s, retry_connect in %d ms", err, DEFAULT_RECONNECT_TIME);
    is_connected_ = false;
    heartbeat_timer_.cancel();
    int retry_delay = DEFAULT_RECONNECT_TIME;
    post(io_context_, [this, retry_delay]()
         {
        std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay));
        start_connect(); });
}

void LogClientImpl::do_async_read()
{
    asio::async_read_until(socket_, asio::dynamic_buffer(read_msg_), '\n', [this](std::error_code ec, std::size_t len)
                           {
        THROW_C3LOG_VERBOSE("do_async_read len: %ld", len);
        if (!ec)
        {
            if (on_recv_)
            {
                on_recv_(this, read_msg_.data(), read_msg_.length());
            }

            read_msg_.erase(0, len);
            do_async_read();
        }
        else if (ec == asio::error::eof)
        {
            retry_connect("do_async_read");
        }
        else
        {
            socket_.close();
        } });
}

void LogClientImpl::do_heartbeat()
{
    static uint32_t heartbeat_sequence = 0;
    static log_msg msg;
    msg.set_sequence(++heartbeat_sequence);

    // Start an asynchronous operation to send a heartbeat message.
    asio::async_write(socket_, asio::buffer(msg.to_string()),
                      [this](std::error_code ec, std::size_t len)
                      {
                          if (!is_connected_)
                              return;
                          if (ec)
                              THROW_C3LOG_VERBOSE("do_heartbeat error: %s", ec.message().c_str());
                          else
                              THROW_C3LOG_VERBOSE("do_heartbeat: %s", msg.data());

                          heartbeat_timer_.expires_after(std::chrono::milliseconds(DEFAULT_RECONNECT_TIME));
                          heartbeat_timer_.async_wait(std::bind(&LogClientImpl::do_heartbeat, this));
                      });
}

////////////////// impl for LogClient //////////////////
LogClient::LogClient(const std::string &host, const std::string &port) : pimpl_(nullptr)
{
    pimpl_ = new LogClientImpl(host, port);
}

LogClient::~LogClient()
{
    if (pimpl_ != nullptr)
        delete pimpl_;
}

void LogClient::start()
{
    if (pimpl_ != nullptr)
        pimpl_->start();
}

void LogClient::write(const std::string &msg)
{
    if (pimpl_)
        pimpl_->async_write(msg);
}

void LogClient::set_callback(OnRecvCallback func)
{
    if (pimpl_)
        pimpl_->set_callback(func);
}

//----------------------------------------------------------------------

static int action(void *self, const char *data, int size)
{
    LogClientImpl *client = static_cast<LogClientImpl *>(self);

    log_msg msg(data, size);
    char type = msg.get_type();
    if (type == 'i')
    {
        std::string info = msg.get_online_info();
        THROW_C3LOG_DEBUG("on online reply: %s", info.c_str());
    }
    else if (type == 's')
    {
        uint32_t seq = msg.get_sequence();
        THROW_C3LOG_DEBUG("on heartbeat reply: %d", seq);
    }
    else
    {
        THROW_C3LOG_DEBUG("unknown type: %c", type);
    }

    return 1;
}

int main(int argc, char *argv[])
{
    try
    {
        LogClient cli("127.0.0.1", "66666");
        cli.set_callback(action);
        cli.start();

        char line[1024];
        while (std::cin.getline(line, 1024))
        {
            std::cout << line << std::endl;
            std::string line_str = line + std::string("\n");
            cli.write(line_str);
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
