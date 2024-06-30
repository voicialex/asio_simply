cmake_minimum_required(VERSION 3.10)

link_libraries(Threads::Threads)

add_subdirectory(allocation)

add_subdirectory(timers)

add_subdirectory(chat)

add_subdirectory(timeouts)

add_subdirectory(executors)

add_subdirectory(services)

add_subdirectory(service)