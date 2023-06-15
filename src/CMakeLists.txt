cmake_minimum_required(VERSION 3.10)

project(libphxecho VERSION 1.0.0)
cmake_policy(SET CMP0048 NEW)

# specify the C++ standard & compile flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(Protobuf REQUIRED)
message(STATUS "[LOG] PROTOC BINARY: ${Protobuf_PROTOC_EXECUTABLE}")
execute_process(
  COMMAND
    ${Protobuf_PROTOC_EXECUTABLE} --proto_path=${PROJECT_SOURCE_DIR}/master
    --cpp_out=${PROJECT_SOURCE_DIR}/master
    ${PROJECT_SOURCE_DIR}/master/master_sm.proto)

execute_process(
  COMMAND
    ${Protobuf_PROTOC_EXECUTABLE} --proto_path=${PROJECT_SOURCE_DIR}/comm
    --cpp_out=${PROJECT_SOURCE_DIR}/comm
    ${PROJECT_SOURCE_DIR}/comm/paxos_msg.proto)

message(STATUS "[LOG] PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

# specify sys include & libs
include_directories(BEFORE /usr/include /usr/local/include)
link_directories(BEFORE /usr/lib /usr/local/lib)

# add all the source file in PROJECT_SOURCE_DIR to SOURCES variable
set(LIB_SOURCE_DIRS
    algorithm
    checkpoint
    comm
    communicate
    communicate/tcp
    config
    logstorage
    master
    node
    sm-base
    tools
    utils)
message(STATUS "[LOG] LIB_SOURCE_DIRS: ${LIB_SOURCE_DIRS}")

foreach(dir IN LISTS LIB_SOURCE_DIRS)
  # message(STATUS "[LOG] dir: ${dir}")
  aux_source_directory("${dir}" SOURCES)
endforeach()
message(STATUS "[LOG] SOURCES: ${SOURCES}")

add_library(phxpaxos STATIC ${SOURCES})
foreach(dir IN LISTS LIB_SOURCE_DIRS)
  # message(STATUS "[LOG] dir: ${dir}")
  target_include_directories(phxpaxos PUBLIC "${PROJECT_SOURCE_DIR}/${dir}")
endforeach()
target_include_directories(phxpaxos PUBLIC "${PROJECT_SOURCE_DIR}/../include")
target_link_libraries(
  phxpaxos
  gtest
  gflags
  glog
  gmock
  leveldb
  pthread)

add_subdirectory(ut)
add_subdirectory(test1)
add_subdirectory(benchmark)
