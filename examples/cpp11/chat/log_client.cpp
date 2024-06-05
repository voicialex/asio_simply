
#include <deque>
#include <iostream>
#include <string>
#include <thread>
#ifdef __linux__
#include <pthread.h>
#endif
#include <functional>
#include "log_message.hpp"
#include "asio.hpp"

#define ENABLE_HEARTBEAT

typedef std::function<int(void *, const char *, int)> OnRecvCallback;

class LogClientImpl;
class LogClient
{
  public:
    LogClient(const std::string &host, const std::string &port);

    ~LogClient();

    void start();

    void write(const log_message &msg);

    void set_callback(OnRecvCallback func = OnRecvCallback());

  private:
    LogClientImpl *pimpl_;
};

///////////////////////////////////////////

using asio::steady_timer;
using asio::ip::tcp;

using log_message_queue = std::deque<log_message>;

class LogClientImpl
{
  public:
    LogClientImpl(const std::string &host, const std::string &port);

    ~LogClientImpl();

    void write(const log_message &msg);

    void set_callback(OnRecvCallback func = OnRecvCallback());

    void start();

    void close();

  private:
    void start_connect();

    void retry_connect();

    void start_read();

    void do_read_header();

    void do_read_body();

    void do_write();

    void start_heartbeat();

    void send_heartbeat(const std::error_code &error);

  private:
    asio::io_context io_context_;
    tcp::socket socket_;
    tcp::resolver::results_type endpoints_;
    steady_timer heartbeat_timer_;
    log_message read_msg_;
    log_message_queue write_msgs_;
    OnRecvCallback on_recv_;
    std::thread io_thread_;
};

LogClientImpl::LogClientImpl(const std::string &host, const std::string &port)
    : socket_(io_context_), heartbeat_timer_(io_context_)
{
    // 在构造函数内部解析主机和端口
    tcp::resolver resolver(io_context_);
    endpoints_ = resolver.resolve(host, port);
}

LogClientImpl::~LogClientImpl()
{
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

    io_thread_ = std::thread([this]() {
#ifdef __linux__
        pthread_setname_np(pthread_self(), "IOThread.Cli");
#endif
        THROW_C3LOG_VERBOSE("io_context_ running BEGIN ...");
        io_context_.run();
        THROW_C3LOG_VERBOSE("io_context_ running DONE ...");
    });
}

void LogClientImpl::write(const log_message &msg)
{
#ifdef ENABLE_MESSAGE_QUEUE
    asio::post(io_context_, [this, msg]() {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
            do_write();
        }
    });
#else
    asio::async_write(socket_, asio::buffer(msg.data(), msg.length()),
                      [this](std::error_code ec, std::size_t /*length*/) {
                          if (!ec)
                          {
                              // send success
                          }
                          else if (ec == asio::error::eof)
                          {
                              retry_connect();
                          }
                          else
                          {
                              socket_.close();
                          }
                      });
#endif
}

void LogClientImpl::do_write()
{
    asio::async_write(socket_, asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
                      [this](std::error_code ec, std::size_t /*length*/) {
                          if (!ec)
                          {
                              write_msgs_.pop_front();
                              if (!write_msgs_.empty())
                              {
                                  do_write();
                              }
                          }
                          else if (ec == asio::error::eof)
                          {
                              retry_connect();
                          }
                          else
                          {
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
    asio::post(io_context_, [this]() { socket_.close(); });
}

void LogClientImpl::start_connect()
{
    THROW_C3LOG_VERBOSE("start_connect ...");
    asio::async_connect(socket_, endpoints_, [this](std::error_code ec, tcp::endpoint) {
        if (!ec)
        {
            start_read();
#ifdef ENABLE_HEARTBEAT
            start_heartbeat();
#endif
        }
        else
        {
            retry_connect();
        }
    });
}

void LogClientImpl::retry_connect()
{
    THROW_C3LOG_VERBOSE("start_connect failed, retrying in %d ms", DEFAULT_RECONNECT_TIME);
    int retry_delay = DEFAULT_RECONNECT_TIME;
    post(io_context_, [this, retry_delay]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay));
        start_connect();
    });
}

void LogClientImpl::start_read()
{
    do_read_header();
}

void LogClientImpl::do_read_header()
{
    THROW_C3LOG_VERBOSE("do_read_header");
    asio::async_read(socket_, asio::buffer(read_msg_.data(), log_message::header_length),
                     [this](std::error_code ec, std::size_t /*length*/) {
                         if (!ec && read_msg_.decode_header())
                         {
                             do_read_body();
                         }
                         else if (ec == asio::error::eof)
                         {
                             retry_connect();
                         }
                         else
                         {
                             socket_.close();
                         }
                     });
}

void LogClientImpl::do_read_body()
{
    THROW_C3LOG_VERBOSE("do_read_body");
    asio::async_read(socket_, asio::buffer(read_msg_.body(), read_msg_.body_length()),
                     [this](std::error_code ec, std::size_t /*length*/) {
                         if (!ec)
                         {
                             if (on_recv_)
                             {
                                 on_recv_(this, read_msg_.body(), read_msg_.body_length());
                             }
                             read_msg_.clear();
                             do_read_header();
                         }
                         else if (ec == asio::error::eof)
                         {
                             retry_connect();
                         }
                         else
                         {
                             socket_.close();
                         }
                     });
}

void LogClientImpl::start_heartbeat()
{
    THROW_C3LOG_VERBOSE("start_heartbeat");
    c3log_protocol heartbeat_msg;
    heartbeat_msg.clear();
    heartbeat_msg.type(c3log_protocol::Type::heartbeat_req);
    heartbeat_msg.status(c3log_protocol::Status::status_success);
    heartbeat_msg.add_app(__progname);
    heartbeat_msg.encode();

    log_message msg;
    msg.body_length(heartbeat_msg.size());
    std::memcpy(msg.body(), heartbeat_msg.data(), heartbeat_msg.size());
    msg.encode_header();
    printf("start_heartbeat length: %d\n", msg.length());

    // Start an asynchronous operation to send a heartbeat message.
    asio::async_write(socket_, asio::buffer(msg.data(), msg.length()),
                      std::bind(&LogClientImpl::send_heartbeat, this, std::placeholders::_1));
}

void LogClientImpl::send_heartbeat(const std::error_code &error)
{
    THROW_C3LOG_VERBOSE("send_heartbeat: %s", error.message().c_str());
    if (!error)
    {
        heartbeat_timer_.expires_after(std::chrono::milliseconds(DEFAULT_RECONNECT_TIME));
        heartbeat_timer_.async_wait(std::bind(&LogClientImpl::start_heartbeat, this));
    }
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

void LogClient::write(const log_message &msg)
{
    if (pimpl_)
        pimpl_->write(msg);
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

    c3log_protocol proto_msg(data, size);
    proto_msg.decode();
    if (proto_msg.type() == c3log_protocol::Type::request_part)
    {
        if (proto_msg.status() == c3log_protocol::Status::status_success)
        {
            std::vector<std::string> apps = proto_msg.apps();
            for (const auto &app : apps)
            {
                if (strcmp(__progname, app.c_str()) == 0)
                {
                    // correct object, do some action
                    THROW_C3LOG_DEBUG("correct app %s, action!", __progname);
                    proto_msg.clear();
                    proto_msg.type(c3log_protocol::Type::response);
                    proto_msg.status(c3log_protocol::Status::status_success);
                    proto_msg.add_app(__progname);
                    proto_msg.encode();

                    log_message msg;
                    msg.body_length(proto_msg.size());
                    std::memcpy(msg.body(), proto_msg.data(), proto_msg.size());
                    puts("action");
                    msg.encode_header();
                    client->write(msg);
                }
            }
        }
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

        char line[log_message::max_body_length + 1];
        while (std::cin.getline(line, log_message::max_body_length + 1))
        {
            log_message msg;
            msg.body_length(std::strlen(line));
            std::memcpy(msg.body(), line, msg.body_length());
            puts("main");
            msg.encode_header();
            cli.write(msg);
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
