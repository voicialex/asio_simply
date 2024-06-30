
set(TARGET_NAME priority_scheculer)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/priority_scheduler.cpp)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
