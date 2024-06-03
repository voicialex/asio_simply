cmake_minimum_required(VERSION 3.10)

link_libraries(Threads::Threads)


set(TARGET_NAME async_2)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/executors/async_2.cpp)
install(TARGETS async_2 DESTINATION ${CMAKE_INSTALL_PREFIX}/test/asio)

set(TARGET_NAME priority_scheduler)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/executors/priority_scheduler.cpp)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/test/asio)
