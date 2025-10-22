# CMake build configuration
cmake_minimum_required(VERSION 3.15)
project(Task4PC CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Executable dari main.cpp
add_executable(task4
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# Compile serial library (wjwwood/serial)
add_library(serial_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/serial/src/serial.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/serial/src/impl/win.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/serial/src/impl/list_ports/list_ports_win.cc
)

# Include directories
target_include_directories(serial_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/serial/include
)

target_include_directories(task4 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries
target_link_libraries(task4 PRIVATE serial_lib)

# Windows: link ws2_32 dan setupapi untuk serial communication
if (WIN32)
    target_link_libraries(task4 PRIVATE ws2_32 setupapi)
endif()
