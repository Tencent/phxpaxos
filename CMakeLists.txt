cmake_minimum_required(VERSION 3.10)

# specify project name
project(phxpaxos VERSION 1.0.0)
cmake_policy(SET CMP0048 NEW)

# specify the C++ standard & compile flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# print message
message(STATUS "[LOG] PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

# using sys include & libs
include_directories(BEFORE /usr/include /usr/local/include)
link_directories(BEFORE /usr/lib /usr/local/lib)

# specify export include
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/include
                    ${PROJECT_SOURCE_DIR}/plugin/include)

# specify subdirectory
add_subdirectory(src)
add_subdirectory(plugin)
add_subdirectory(sample)
