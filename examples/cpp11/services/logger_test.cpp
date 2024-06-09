#include <asio/ts/executor.hpp>
#include <asio/detail/thread/thread_pool.hpp>
#include <iostream>
#include <string>

#include <asio.hpp>
#include "logger.hpp"

using asio::bind_executor;
using asio::dispatch;
using asio::make_work_guard;
using asio::post;
using asio::thread_pool;

// A function to asynchronously read a single line from an input stream.
template <class Handler>
void async_getline(std::istream &is, Handler handler)
{
    // Create executor_work for the handler's associated executor.
    auto work = make_work_guard(handler);

    asio::execution_context &context = work.get_executor().context();
    services::logger logger(context, "start");
    logger.use_file("log.txt");
    logger.log("running ...");

    std::string line;
    logger.log(line);
    std::getline(is, line);
}

int main(int argc, char *argv[])
{
    thread_pool pool;

    std::cout << "Enter a line: ";

    async_getline(std::cin,
                  bind_executor(pool, [](std::string line)
                                { std::cout << "Line: " << line << "\n"; }));

    pool.join();
}
