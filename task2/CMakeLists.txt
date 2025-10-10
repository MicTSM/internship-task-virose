cmake_minimum_required(VERSION 3.15)

# Project Name
project("RobotMotion")

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(rapidjson_include_dir "${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include/")

include_directories(${rapidjson_include_dir})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# include_directories()

add_executable(task2-virose
    Bos.cpp
    Movie.cpp
    Unit.cpp
)

# target_link_libraries()