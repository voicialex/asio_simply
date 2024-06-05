cmake_minimum_required(VERSION 3.10)

link_libraries(Threads::Threads)

set(TARGET_NAME allocation)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/allocation/server.cpp)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/test/asio)

set(TARGET_NAME timer_one_shot)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/timers/time_t_timer.cpp)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/test/asio)

set(TARGET_NAME timer_period)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/timers/periodic_timer.cpp)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/test/asio)

set(TARGET_NAME server)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/timeouts/server.cpp)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/test/asio)

set(TARGET_NAME async_tcp_client)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/timeouts/async_tcp_client.cpp)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/test/asio)

set(TARGET_NAME priority_scheculer)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/executors/priority_scheduler.cpp)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/test/asio)

set(TARGET_NAME chat_client)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/chat/chat_client.cpp)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/test/asio)

set(TARGET_NAME chat_server)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/chat/chat_server.cpp)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/test/asio)