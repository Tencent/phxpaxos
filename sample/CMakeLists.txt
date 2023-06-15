cmake_minimum_required(VERSION 3.10)

# specify project name
project(phxkv VERSION 1.0.0)
cmake_policy(SET CMP0048 NEW)

# specify the C++ standard & compile flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_subdirectory(phxecho)
add_subdirectory(phxelection)
add_subdirectory(phxkv)
