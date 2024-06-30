
set(TARGET_NAME test_service)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/test_service.cpp)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
