cmake_minimum_required(VERSION 3.10)

project(thread_pool LANGUAGES C CXX VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

find_package(Threads REQUIRED)
add_definitions(-DASIO_STANDALONE)
add_definitions(-DASIO_HAS_PTHREADS)
# add_definitions(-DASIO_NO_DEPRECATED)
# ASIO_HAS_CHRONO  启用std::chrono支
# ASIO_HAS_VARIADIC_TEMPLATES 宏是用来指示编译器是否支持可变模板参数的宏定义
# ASIO_HAS_CXX11_ALLOCATORS

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/build/test)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp11)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp14)
