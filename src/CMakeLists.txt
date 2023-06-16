cmake_minimum_required(VERSION 3.10)

project(libphxecho VERSION 1.0.0)
cmake_policy(SET CMP0048 NEW)

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

  # target_include_directories(phxpaxos PUBLIC "${PROJECT_SOURCE_DIR}/${dir}")
  include_directories(phxpaxos PUBLIC "${PROJECT_SOURCE_DIR}/${dir}")
endforeach()

message(STATUS "[LOG] CMAKE_LIBRARY_PATH: ${CMAKE_LIBRARY_PATH}")

add_subdirectory(ut)
add_subdirectory(test1)
add_subdirectory(benchmark)
