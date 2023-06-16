cmake_minimum_required(VERSION 3.10)

project(phxecho VERSION 1.0.0)
cmake_policy(SET CMP0048 NEW)

# specify the C++ standard & compile flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

message(STATUS "[LOG/phxecho] PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

aux_source_directory(${PROJECT_SOURCE_DIR}/ SOURCES)
message(STATUS "[LOG/phxecho] SOURCES: ${SOURCES}")

add_executable(phxecho ${SOURCES})
target_link_libraries(
  phxecho
  ${CMAKE_BINARY_DIR}/src/libphxpaxos.a
  ${CMAKE_BINARY_DIR}/plugin/libplugin.a
  protobuf
  leveldb
  glog
  gflags)
