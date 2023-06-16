cmake_minimum_required(VERSION 3.10)

# specify project name
project(phxkv VERSION 1.0.0)
cmake_policy(SET CMP0048 NEW)

# print message
message(STATUS "[LOG/phxkv] PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
find_package(Protobuf REQUIRED)
message(STATUS "[LOG/phxkv] PROTOC BINARY: ${Protobuf_PROTOC_EXECUTABLE}")
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
message(STATUS "[LOG/phxkv] GRPC_CPP_PLUGIN: ${GRPC_CPP_PLUGIN}")
execute_process(
  COMMAND
    ${Protobuf_PROTOC_EXECUTABLE} --proto_path=${PROJECT_SOURCE_DIR}
    --grpc_out=${PROJECT_SOURCE_DIR}
    --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN} --cpp_out=${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/phxkv.proto)

aux_source_directory(${PROJECT_SOURCE_DIR} SOURCES)
list(REMOVE_ITEM SOURCES ${PROJECT_SOURCE_DIR}/kv_grpc_client_main.cpp)
list(REMOVE_ITEM SOURCES ${PROJECT_SOURCE_DIR}/kv_grpc_server_main.cpp)
message(STATUS "[LOG/phxkv] SOURCES: ${SOURCES}")

add_executable(phxkv_grpc_client ${PROJECT_SOURCE_DIR}/kv_grpc_client_main.cpp
                                 ${SOURCES})
add_dependencies(phxkv_grpc_client phxpaxos plugin)
target_link_libraries(
  phxkv_grpc_client
  ${CMAKE_BINARY_DIR}/src/libphxpaxos.a
  ${CMAKE_BINARY_DIR}/plugin/libplugin.a
  pthread
  grpc++
  glog
  leveldb
  protobuf)

add_executable(phxkv_grpc_server ${PROJECT_SOURCE_DIR}/kv_grpc_server_main.cpp
                                 ${SOURCES})
add_dependencies(phxkv_grpc_server phxpaxos plugin)
target_link_libraries(
  phxkv_grpc_server
  ${CMAKE_BINARY_DIR}/src/libphxpaxos.a
  ${CMAKE_BINARY_DIR}/plugin/libplugin.a
  pthread
  grpc++
  glog
  leveldb
  protobuf)
