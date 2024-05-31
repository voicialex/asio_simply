cmake_minimum_required(VERSION 3.10)

add_executable(timers ${CMAKE_CURRENT_SOURCE_DIR}/timers/time_t_timer.cpp)
target_link_libraries(timers PUBLIC Threads::Threads)
install(TARGETS timers DESTINATION ${CMAKE_INSTALL_PREFIX}/test/asio)


# add_executable(chat_client ${CMAKE_CURRENT_SOURCE_DIR}/chat/chat_client.cpp)
# target_link_libraries(chat_client PUBLIC Threads::Threads)
# install(TARGETS chat_client DESTINATION ${CMAKE_INSTALL_PREFIX}/test/asio)

# add_executable(chat_server ${CMAKE_CURRENT_SOURCE_DIR}/chat/chat_server.cpp)
# target_link_libraries(chat_server PUBLIC Threads::Threads)
# install(TARGETS chat_server DESTINATION ${CMAKE_INSTALL_PREFIX}/test/asio)