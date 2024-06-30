
#include "logger.hpp"
#include <asio.hpp>

int main()
{
    asio::io_context io_context;
    services::logger logger(io_context, "start_tag");
    logger.use_file("log.txt");
    logger.log("aaa");

    // std::this_thread::sleep_for(std::chrono::seconds(1));

    while(1)
    {
        std::string line;
        std::cin >> line;
        logger.log(line);
    }
}
