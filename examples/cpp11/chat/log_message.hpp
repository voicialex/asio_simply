
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
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

class log_msg
{
  public:
    log_msg(std::string &str) : str_(str)
    {
    }
    log_msg(const char *str, int len) : str_(str, len)
    {
    }
    log_msg() : str_("")
    {
    }

    void set_type(char c)
    {
        if (str_.size() >= 1)
        {
            str_[0] = c;
        }
    }
    char get_type()
    {
        if (str_.size() >= 1)
        {
            return str_[0];
        }
        return 'u'; // unknown
    }

    void set_sequence(uint32_t seq)
    {
        // 预留足够的空间用于序列化数据
        str_.clear();
        str_ = "s:" + std::to_string(seq) + std::string("\n");
    }
    uint32_t get_sequence()
    {
        uint32_t seq = 0;
        // 检查字符串是否至少包含 "s:" 加上一个数字和一个换行符的最小长度
        if (str_.size() < 4)
        {
            return seq; // 如果字符串太短，则返回0
        }
        if (str_[0] != 's' || str_[1] != ':')
        {
            return seq; // 如果不是，返回0
        }

        // 尝试从第三个字符开始解析数字字符串
        std::string sequence_str = str_.substr(2);
        try
        {
            seq = std::stoul(sequence_str); // 将字符串转换为无符号长整数
        }
        catch (const std::invalid_argument &ia)
        {
            // 如果转换失败，则返回0
        }
        catch (const std::out_of_range &oor)
        {
            // 如果数值超出范围，同样返回0
        }

        return seq;
    }

    void set_online_info()
    {
        str_.clear();
        str_ = "i:" + std::string(__progname) + std::string("\n");
    }
    std::string get_online_info()
    {
        std::string info("");
        size_t start_pos = 2;
        if (start_pos >= str_.size())
        {
            return info;
        }
        
        if (str_[0] != 'i' || str_[1] != ':')
        {
            return info; // 如果不是，返回0
        }
    
        size_t end_pos = str_.find('\n', start_pos);
        if (end_pos == std::string::npos)
        {
            return info;
        }
        info = str_.substr(start_pos, end_pos - start_pos);
        return info;
    }

    const char *data()
    {
        return str_.data();
    }
    uint32_t size()
    {
        return str_.size();
    }

    std::string &to_string()
    {
        return str_;
    }

  private:
    std::string str_;
};

#endif
