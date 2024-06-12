#include <deque>
#include <iostream>
#include <set>
#include <thread>
#ifdef __linux__
#include <pthread.h>
#endif
#include "asio.hpp"
#include "log_message.hpp"
#include <functional>

typedef std::function<int(void *, const char *, int)> OnRecvCallback;

class LogServerImpl;
class LogServer
{
  public:
    LogServer(const std::string &host, const std::string &port);

    ~LogServer();

    void start();

    void broadcast(const std::string &msg);

    void set_callback(OnRecvCallback func = OnRecvCallback());

  private:
    LogServerImpl *pimpl_;
};

//////////////////////////////////////

using asio::ip::tcp;

class LogChannel;

class LogSession : public std::enable_shared_from_this<LogSession>
{
  public:
    LogSession(tcp::socket socket, LogChannel &room);

    ~LogSession();

    void start();

    void set_callback(OnRecvCallback func = OnRecvCallback());

    void set_remote_name(std::string name);

    void async_write(const std::string &msg);

    std::string session_info();

  private:
    void do_async_read();

    void do_async_write();

    tcp::socket socket_;
    LogChannel &channel_;
    std::string name_;
    std::string ip_port_;
    std::string read_msg_;
    std::deque<std::string> write_msgs_;
    OnRecvCallback on_recv_;
};

using LogSessionPtr = std::shared_ptr<LogSession>;

//----------------------------------------------------------------------

class LogChannel
{
  public:
    void join(LogSessionPtr session)
    {
        sessions_.insert(session);
    }

    void leave(LogSessionPtr session)
    {
        sessions_.erase(session);
    }

    void deliver(const std::string &msg)
    {
        for (auto session : sessions_)
            session->async_write(msg);
    }

    void print_member_info()
    {
        for (auto session : sessions_)
        {
            THROW_C3LOG_VERBOSE("member_info --> %s", session->session_info().c_str());
        }
    }

  private:
    std::set<LogSessionPtr> sessions_;
    enum
    {
        max_recent_msgs = 100
    };
};

class LogServerImpl
{
  public:
    LogServerImpl(const std::string &host, const std::string &port);

    ~LogServerImpl();

    void start();

    void broadcast(const std::string &msg);

    void set_callback(OnRecvCallback func = OnRecvCallback());

  private:
    void do_accept();

    asio::io_context io_context_;
    tcp::acceptor acceptor_;
    tcp::endpoint endpoint_;
    LogChannel channel_;
    OnRecvCallback on_session_recv_;
    std::thread io_thread_;
};

//----------------------------------------------------------------------

LogSession::LogSession(tcp::socket socket, LogChannel &room)
    : socket_(std::move(socket)), channel_(room), name_("unknown"), ip_port_(std::string())
{
    tcp::endpoint endpoint = socket_.remote_endpoint();
    ip_port_ = endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
    THROW_C3LOG_VERBOSE("new session : %s", session_info().c_str());
}

LogSession::~LogSession()
{
    THROW_C3LOG_VERBOSE("del session : %s", session_info().c_str());
}

void LogSession::start()
{
    channel_.join(shared_from_this());
    do_async_read();
}

std::string LogSession::session_info()
{
    return ip_port_ + " (" + name_ + ")";
}

void LogSession::set_callback(OnRecvCallback func)
{
    if (func)
        on_recv_ = func;
}

void LogSession::set_remote_name(std::string name)
{
    name_ = name;
}

void LogSession::async_write(const std::string &msg)
{
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress)
    {
        do_async_write();
    }
}

void LogSession::do_async_write()
{
    auto self(shared_from_this());
    asio::async_write(socket_, asio::buffer(write_msgs_.front()),
                      [this, self](std::error_code ec, std::size_t /*length*/) {
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
                              channel_.leave(shared_from_this());
                          }
                      });
}

void LogSession::do_async_read()
{
    THROW_C3LOG_VERBOSE("do_async_read");
    auto self(shared_from_this());
    asio::async_read_until(socket_, asio::dynamic_buffer(read_msg_), '\n',
                           [this, self](std::error_code ec, std::size_t len) {
                               if (!ec)
                               {
                                   if (on_recv_)
                                       on_recv_(self.get(), read_msg_.data(), read_msg_.size());

                                   read_msg_.erase(0, len);
                                   do_async_read();
                               }
                               else
                               {
                                   THROW_C3LOG_DEBUG("app leave, reason: %s\n", ec.message().c_str());
                                   channel_.leave(shared_from_this());
                               }
                           });
}

//----------------------------------------------------------------------

LogServerImpl::LogServerImpl(const std::string &host, const std::string &port)
    : acceptor_(io_context_), endpoint_(asio::ip::make_address(host), std::stoi(port))
{
    acceptor_.open(endpoint_.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint_);
    acceptor_.listen();
    do_accept();
}

LogServerImpl::~LogServerImpl()
{
    io_context_.stop();
    if (io_thread_.joinable())
    {
        io_thread_.join();
    }
}

void LogServerImpl::start()
{
    io_thread_ = std::thread([this]() {
#ifdef __linux__
        pthread_setname_np(pthread_self(), "IOThread.Svr");
#endif
        io_context_.run();
    });
}

void LogServerImpl::broadcast(const std::string &msg)
{
    channel_.deliver(msg);
}

void LogServerImpl::set_callback(OnRecvCallback func)
{
    if (func)
    {
        on_session_recv_ = func;
    }
}

void LogServerImpl::do_accept()
{
    acceptor_.async_accept([this](std::error_code ec, tcp::socket socket) {
        if (!ec)
        {
            auto session = std::make_shared<LogSession>(std::move(socket), channel_);
            session->start();
            if (on_session_recv_)
                session->set_callback(on_session_recv_);
        }
        else
        {
            THROW_C3LOG_EXCEPTION("Error in async_accept: %s", ec.message().c_str());
        }

        do_accept();
    });
}

////////////////// impl for LogServer //////////////////
LogServer::LogServer(const std::string &host, const std::string &port) : pimpl_(nullptr)
{
    pimpl_ = new LogServerImpl(host, port);
}

LogServer::~LogServer()
{
    if (pimpl_ != nullptr)
        delete pimpl_;
}

void LogServer::start()
{
    if (pimpl_ != nullptr)
        pimpl_->start();
}

void LogServer::broadcast(const std::string &msg)
{
    if (pimpl_)
        pimpl_->broadcast(msg);
}

void LogServer::set_callback(OnRecvCallback func)
{
    if (pimpl_)
        pimpl_->set_callback(func);
}

//----------------------------------------------------------------------

static int action(void *self, const char *data, int size)
{
    LogSession *session = static_cast<LogSession *>(self);

    log_msg msg(data, size);
    char type = msg.get_type();
    if (type == 'i')
    {
        std::string info = msg.get_online_info();
        session->set_remote_name(info);
        THROW_C3LOG_DEBUG("on online: %s", session->session_info().c_str());

        log_msg msg_reply;
        msg_reply.set_online_info();
        session->async_write(msg_reply.to_string());
    }
    else if (type == 's')
    {
        uint32_t seq = msg.get_sequence();
        THROW_C3LOG_DEBUG("on heartbeat: %d", seq);
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
        LogServer svr("127.0.0.1", "66666");
        svr.set_callback(action);
        svr.start();

        char line[1024];
        while (std::cin.getline(line, 1024))
        {
            std::cout << line << std::endl;
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
