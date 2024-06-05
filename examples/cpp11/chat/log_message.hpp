
#ifndef LOG_MESSAGE_HPP
#define LOG_MESSAGE_HPP

#include <stdio.h>
extern char *__progname;

// #define THROW_C3LOG_EXCEPTION(fmt, ...) C3LOG_FMT_LEVEL(C3LOG, LogLevel::C3LOG_LV_ERROR, true, fmt, ##__VA_ARGS__)

// #define THROW_EXCEPTION(tag, fmt, ...) C3LOG_FMT_LEVEL(tag, LogLevel::C3LOG_LV_ERROR, true, fmt, ##__VA_ARGS__)

#define THROW_LOG_OUTPUT(tag, lv, fmt, ...) printf(#tag "." #lv "(%s): " fmt "\n", __progname, ##__VA_ARGS__)

#define THROW_C3LOG_EXCEPTION(fmt, ...) THROW_LOG_OUTPUT(c3log, e, fmt, ##__VA_ARGS__)

#define THROW_C3LOG_INFO(fmt, ...) THROW_LOG_OUTPUT(c3log, i, fmt, ##__VA_ARGS__)

#define THROW_C3LOG_DEBUG(fmt, ...) THROW_LOG_OUTPUT(c3log, d, fmt, ##__VA_ARGS__)

#define THROW_C3LOG_VERBOSE(fmt, ...) THROW_LOG_OUTPUT(c3log, v, fmt, ##__VA_ARGS__)

#define DEFAULT_RECONNECT_TIME 3000

////////////////////////////////////////

#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <iterator>

class log_message
{
public:
  enum { header_length = 4 };
  enum { max_body_length = 512 };

  log_message() : body_length_(0) {}

  const char *data() const { return data_; }

  char *data() { return data_; }

  int length() const { return header_length + body_length_; }

  const char *body() const { return data_ + header_length; }

  char *body() { return data_ + header_length; }

  int body_length() const { return body_length_; }

  void body_length(int new_length)
  {
    body_length_ = new_length;
    if (body_length_ > max_body_length)
      body_length_ = max_body_length;
  }

  void clear() { memset(data_, 0, header_length + max_body_length); }

  bool decode_header()
  {
    char header[header_length + 1] = "";
    std::strncat(header, data_, header_length);
    body_length_ = std::atoi(header);
    printf("decode_header body_length_: %d\n", body_length_);
    if (body_length_ > max_body_length || body_length_ <= 0)
    {
      body_length_ = 0;
      return false;
    }
    return true;
  }

  void encode_header()
  {
    printf("encode_header body_length_: %d\n", body_length_);
    char header[header_length + 1] = "";
    std::sprintf(header, "%4d", static_cast<int>(body_length_));
    std::memcpy(data_, header, header_length);
  }

private:
  char data_[header_length + max_body_length];
  int body_length_;
};

class c3log_protocol
{
public:
  static const size_t kCheapPrepend = 4;
  static const size_t kInitialSize = 512;

  c3log_protocol(size_t initialSize = kInitialSize)
      : buffer_(initialSize + kCheapPrepend)
  {
  }

  c3log_protocol(const char *data, size_t size)
  {
    buffer_.reserve(size);
    buffer_.insert(buffer_.end(), data, data + size);
  }

  enum Type : uint8_t
  {
    type_invalid = 0,
    request_part = 1,
    request_all = 2,
    response = 3,
    heartbeat_req = 4,
    heartbeat_rep = 5,
  };

  enum Status : uint8_t
  {
    status_invalid = 0,
    status_failed = 1,
    status_success = 2,
  };

  Type type() { return type_; }

  void type(Type t) { type_ = t; }

  std::vector<std::string> apps() { return apps_; }

  void add_app(std::string app) { apps_.push_back(app); }

  Status status() { return status_; }

  void status(Status s) { status_ = s; }

  int size() { return buffer_.size(); }

  char *data() { return buffer_.data(); }

  void encode()
  {
    buffer_.clear();
    buffer_.reserve(kCheapPrepend + cal_apps_size());
    buffer_.push_back(static_cast<char>(type_));
    buffer_.push_back(static_cast<char>(status_));
    buffer_.push_back('\0');
    buffer_.push_back('\0');

    encodeString(apps_, buffer_);
  }

  void decode()
  {
    if (buffer_.size() > kCheapPrepend)
    {
      type_ = static_cast<Type>(buffer_[0]);
      status_ = static_cast<Status>(buffer_[1]);

      apps_.clear();
      decodeString(buffer_, apps_);
    }
  }

  void clear()
  {
    buffer_.clear();
    apps_.clear();
    type_ = type_invalid;
    status_ = status_invalid;
  }

private:
  const char *begin() const { return &*buffer_.begin(); }

  char *begin() { return &*buffer_.begin(); }

  int cal_apps_size()
  {
    int total_size = 0;
    for (const auto &app : apps_)
    {
      total_size += app.size() + 1;
    }
    return total_size;
  }

  void encodeString(const std::vector<std::string> &apps, std::vector<char> &buffer)
  {
    for (const auto &app : apps)
    {
      // 将当前字符串的内容复制到 buffer 的末尾
      std::copy(app.begin(), app.end(), std::back_inserter(buffer));
      buffer.push_back(',');
    }

    // 删除最后一个逗号，因为最后一个字符串不需要跟逗号
    if (!buffer.empty())
    {
      buffer.pop_back();
    }

    // printf("buffer: %s\n", buffer.data() + kCheapPrepend);
  }

  void decodeString(std::vector<char> &buffer, std::vector<std::string> &apps)
  {
    std::stringstream ss;
    ss.str(std::string(buffer.begin() + kCheapPrepend, buffer.end()));

    std::string token;
    while (std::getline(ss, token, ','))
    {
      apps.push_back(token);
    }
  }

private:
  enum Type type_ = {type_invalid};
  Status status_ = {status_invalid};
  std::vector<std::string> apps_;

  std::vector<char> buffer_;
};

#endif
