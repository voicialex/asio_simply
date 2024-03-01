cmake_minimum_required(VERSION 3.10)

add_executable(async_2 ${CMAKE_CURRENT_SOURCE_DIR}/executors/async_2.cpp)
target_link_libraries(async_2 PUBLIC Threads::Threads)

install(TARGETS async_2 DESTINATION ${CMAKE_INSTALL_PREFIX}/test/asio)
