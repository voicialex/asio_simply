
set(TARGET_NAME timer_one_shot)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/time_t_timer.cpp)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

set(TARGET_NAME timer_period)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/periodic_timer.cpp)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
