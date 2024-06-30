
set(TARGET_NAME daytime_client)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/daytime_client.cpp)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

set(TARGET_NAME logger_test)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/logger_test.cpp)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
