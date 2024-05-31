cmake_minimum_required(VERSION 3.10)

project(thread_pool LANGUAGES C CXX VERSION 1.0)

find_package(Threads REQUIRED)

add_definitions(-DASIO_STANDALONE)
add_definitions(-DASIO_HAS_PTHREADS)
add_definitions(-DASIO_NO_DEPRECATED)
# ASIO_HAS_CHRONO  启用std::chrono支

include_directories(./include)

# test timer
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp11)

# test async
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp14)
