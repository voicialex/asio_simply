#include <iostream>
#include <asio.hpp>

class PeriodicTimer {
public:
    PeriodicTimer(asio::io_context& io_context, int interval)
        : timer_(io_context, std::chrono::seconds(interval)),
          interval_(interval) {
        start_timer();
    }

private:
    void start_timer() {
        timer_.async_wait([this](const asio::error_code& error) {
            if (!error) {
                std::cout << "Timer triggered!" << std::endl;
                // 重新启动定时器
                timer_.expires_at(timer_.expiry() + std::chrono::seconds(interval_));
                start_timer();
            } else {
                std::cerr << "Timer error: " << error.message() << std::endl;
            }
        });
    }

    asio::steady_timer timer_;
    int interval_;
};

int main() {
    try {
        asio::io_context io_context;
        PeriodicTimer pt(io_context, 2); // 每2秒触发一次
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
