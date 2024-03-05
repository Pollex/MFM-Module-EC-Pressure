cmake_minimum_required(VERSION 3.11)

project(mfm-sensor-module)

# Default ATTiny814 speed
add_compile_definitions(F_CPU=3333333UL TWI_CMD_COUNT=3)
get_filename_component(C_COMPILER_DIR ${CMAKE_C_COMPILER} DIRECTORY)
set(CMAKE_FIND_ROOT_PATH "${C_COMPILER_DIR}/../avr")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_SYSTEM_INCLUDE_PATH "${CMAKE_FIND_ROOT_PATH}/include")
set(CMAKE_SYSTEM_LIBRARY_PATH "${CMAKE_FIND_ROOT_PATH}/lib")

message(STATUS "Set CMAKE_FIND_ROOT_PATH to ${CMAKE_FIND_ROOT_PATH}")
message(STATUS "Set CMAKE_SYSTEM_INCLUDE_PATH to ${CMAKE_SYSTEM_INCLUDE_PATH}")
message(STATUS "Set CMAKE_SYSTEM_LIBRARY_PATH to ${CMAKE_SYSTEM_LIBRARY_PATH}")


include_directories(include)
add_subdirectory(src)

add_avr_executable(${PROJECT_NAME} main.c)
avr_target_link_libraries(${PROJECT_NAME} mod_drivers mod_perif mod_mcu mod_os)
#avr_target_compile_definitions(${PROJECT_NAME} PUBLIC F_CPU=3333333UL)
