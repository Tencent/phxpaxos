cmake_minimum_required(VERSION 3.10)

# specify project name
project(plugin VERSION 1.0.0)
cmake_policy(SET CMP0048 NEW)

# specify the C++ standard & compile flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# print message
message(STATUS "[LOG/plugin] PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

# specify sys include & libs
include_directories(BEFORE /usr/include /usr/local/include)

link_directories(BEFORE /usr/lib /usr/local/lib)

aux_source_directory(${PROJECT_SOURCE_DIR}/logger_google SOURCES)
aux_source_directory(${PROJECT_SOURCE_DIR}/monitor SOURCES)
message(STATUS "[LOG/plugin] SOURCES: ${SOURCES}")

add_library(plugin ${SOURCES})
target_link_libraries(plugin glog)
target_include_directories(
  plugin
  PUBLIC ${PROJECT_SOURCE_DIR}/../include
  PUBLIC ${PROJECT_SOURCE_DIR}/include)
