cmake_minimum_required(VERSION 3.10)

add_executable(timers ${CMAKE_CURRENT_SOURCE_DIR}/timers/time_t_timer.cpp)
target_link_libraries(timers PUBLIC Threads::Threads)

install(TARGETS timers DESTINATION ${CMAKE_INSTALL_PREFIX}/test/asio)
