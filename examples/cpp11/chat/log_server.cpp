#include <set>
#include <deque>
#include <iostream>
#include <thread>
#ifdef __linux__
#include <pthread.h>
#endif
#include <functional>
#include "log_message.hpp"
#include "asio.hpp"

typedef std::function<int(void*, const char*, int)> OnRecvCallback;

class LogServerImpl;
class LogServer
{
public:
  LogServer(const std::string& host, const std::string& port);

  ~LogServer();

  void start();

  void broadcast(const log_message& msg);

  void set_callback(OnRecvCallback func = OnRecvCallback());

private:
  LogServerImpl* pimpl_;
};

//////////////////////////////////////

using asio::ip::tcp;

using log_message_queue = std::deque<log_message>;

class LogParticipant
{
public:
  LogParticipant(tcp::socket socket) : socket_(std::move(socket)) {}

  virtual ~LogParticipant() {}

  virtual void deliver(const log_message& msg) = 0;

  tcp::socket& get_socket() { return socket_; }

protected:
  tcp::socket socket_;
};

using LogParticipantPtr = std::shared_ptr<LogParticipant>;

//----------------------------------------------------------------------

class LogRoom
{
public:
  void join(LogParticipantPtr participant)
  {
    participants_.insert(participant);
    for (auto msg: recent_msgs_)
      participant->deliver(msg);
  }

  void leave(LogParticipantPtr participant)
  {
    participants_.erase(participant);
  }

  void deliver(const log_message& msg)
  {
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs)
      recent_msgs_.pop_front();

    for (auto participant: participants_)
      participant->deliver(msg);
  }

  void print_member_info()
  {
      for (auto participant: participants_)
      {
        tcp::endpoint endpoint = participant->get_socket().remote_endpoint();
        THROW_C3LOG_VERBOSE("member_info --> %s:%d",
              endpoint.address().to_string().c_str(),
              endpoint.port());
      }
  }

private:
  std::set<LogParticipantPtr> participants_;
  enum { max_recent_msgs = 100 };
  log_message_queue recent_msgs_;
};

//----------------------------------------------------------------------

class LogSession
  : public LogParticipant,
    public std::enable_shared_from_this<LogSession>
{
public:
  LogSession(tcp::socket socket, LogRoom& room);

  void start();

  void set_callback(OnRecvCallback func = OnRecvCallback());

  void deliver(const log_message& msg);

private:
  void start_read();

  void do_read_header();

  void do_read_body();

  void do_write();

  LogRoom& room_;
  log_message read_msg_;
  log_message_queue write_msgs_;
  OnRecvCallback on_recv_;
};

class LogServerImpl
{
public:
  LogServerImpl(const std::string& host, const std::string& port);

  ~LogServerImpl();

  void start();

  void broadcast(const log_message& msg);

  void set_callback(OnRecvCallback func = OnRecvCallback());

private:
  void do_accept();

  asio::io_context io_context_;
  tcp::acceptor acceptor_;
  tcp::endpoint endpoint_;
  LogRoom room_;
  OnRecvCallback on_sync_;
  std::thread io_thread_;
};

LogSession::LogSession(tcp::socket socket, LogRoom& room)
          : LogParticipant(std::move(socket)),
            room_(room) {}

void LogSession::start()
{
  room_.join(shared_from_this());
  room_.print_member_info();
  start_read();
}

void LogSession::set_callback(OnRecvCallback func)
{
  if(func)
    on_recv_ = func;
}

void LogSession::deliver(const log_message& msg)
{
  bool write_in_progress = !write_msgs_.empty();
  write_msgs_.push_back(msg);
  if (!write_in_progress)
  {
    do_write();
  }
}

void LogSession::do_write()
{
  auto self(shared_from_this());
  asio::async_write(socket_,
      asio::buffer(write_msgs_.front().data(),
        write_msgs_.front().length()),
      [this, self](std::error_code ec, std::size_t /*length*/)
      {
        if (!ec)
        {
          write_msgs_.pop_front();
          if (!write_msgs_.empty())
          {
            do_write();
          }
        }
        else
        {
          THROW_C3LOG_EXCEPTION("Error in async_write: %s", ec.message().c_str());
          room_.leave(shared_from_this());
        }
      });
}

void LogSession::start_read()
{
  do_read_header();
}

void LogSession::do_read_header()
{
  auto self(shared_from_this());
  asio::async_read(socket_,
      asio::buffer(read_msg_.data(), log_message::header_length),
      [this, self](std::error_code ec, std::size_t /*length*/)
      {
        bool decode_res = read_msg_.decode_header();
        printf("decode_header --> %d **\n", decode_res);
        if (!ec && decode_res)
        {
          do_read_body();
        }
        else
        {
          THROW_C3LOG_DEBUG("app leave %d \n", decode_res);
          room_.leave(shared_from_this());
        }
      });
}

static void read_debug(const char* data, int size)
{
  c3log_protocol proto_msg(data, size);
  proto_msg.decode();
  if (proto_msg.type() == c3log_protocol::Type::heartbeat_req)
  {
    if (proto_msg.status() == c3log_protocol::Status::status_success)
    {
      std::vector<std::string> apps = proto_msg.apps();
      for (const auto &app : apps)
      {
        THROW_C3LOG_DEBUG("app %s, heartbeat!", app.c_str());
      }
    }
  }
}

void LogSession::do_read_body()
{
  auto self(shared_from_this());
  asio::async_read(socket_,
      asio::buffer(read_msg_.body(), read_msg_.body_length()),
      [this, self](std::error_code ec, std::size_t /*length*/)
      {
        if (!ec)
        {
          if (on_recv_)
            on_recv_(self.get(), read_msg_.body(), read_msg_.body_length());

          // read_debug(read_msg_.body(), read_msg_.body_length());

          read_msg_.clear();
          do_read_header();
        }
        else
        {
          THROW_C3LOG_EXCEPTION("Error in async_read: %s", ec.message().c_str());
          room_.leave(shared_from_this());
        }
      });
}

//----------------------------------------------------------------------


LogServerImpl::LogServerImpl(const std::string& host, const std::string& port)
    : acceptor_(io_context_),
      endpoint_(asio::ip::make_address(host), std::stoi(port))
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

void LogServerImpl::broadcast(const log_message& msg)
{
  room_.deliver(msg);
}

void LogServerImpl::set_callback(OnRecvCallback func)
{
  if(func)
  {
    on_sync_ = func;
  }
}

void LogServerImpl::do_accept()
{
  acceptor_.async_accept(
      [this](std::error_code ec, tcp::socket socket)
      {
        if (!ec)
        {
          auto session = std::make_shared<LogSession>(std::move(socket), room_);
          session->start();
          if (on_sync_)
            session->set_callback(on_sync_);
        }
        else
        {
          THROW_C3LOG_EXCEPTION("Error in async_accept: %s", ec.message().c_str());
        }

        do_accept();
      });
}

////////////////// impl for LogServer //////////////////
LogServer::LogServer(const std::string& host, const std::string& port)
  : pimpl_(nullptr)
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

void LogServer::broadcast(const log_message& msg)
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

static int action(void *self, const char* data, int size)
{
  LogServer* server = static_cast<LogServer*>(self);

  c3log_protocol proto_msg(data, size);
  proto_msg.decode();
  if (proto_msg.type() == c3log_protocol::Type::response)
  {
    if (proto_msg.status() == c3log_protocol::Status::status_success)
    {
      std::vector<std::string> apps = proto_msg.apps();
      for (const auto &app : apps)
      {
        THROW_C3LOG_DEBUG("app done %s, action!", app.c_str());
      }
    }
  }
  THROW_C3LOG_DEBUG("type(%d) status(%d)", proto_msg.type(), proto_msg.status());

  return 1;
}

int main(int argc, char* argv[])
{
  try
  {
    LogServer svr("127.0.0.1", "66666");
    svr.set_callback(action);
    svr.start();

    char line[log_message::max_body_length + 1];
    while (std::cin.getline(line, log_message::max_body_length + 1))
    {
      std::string app(line, std::strlen(line));
    
      c3log_protocol proto_msg;
      proto_msg.type(c3log_protocol::Type::request_part);
      proto_msg.status(c3log_protocol::Status::status_success);
      proto_msg.add_app(app);
      proto_msg.add_app("client1");
      proto_msg.add_app("client2");
      proto_msg.add_app("client3");
      proto_msg.encode();

      log_message msg;
      msg.body_length(proto_msg.size());
      std::memcpy(msg.body(), proto_msg.data(), proto_msg.size());
      msg.encode_header();
      svr.broadcast(msg);
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
