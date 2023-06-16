cmake_minimum_required(VERSION 3.10)

project(phxecho VERSION 1.0.0)
cmake_policy(SET CMP0048 NEW)

message(STATUS "[LOG/phxecho] PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

aux_source_directory(. SOURCES)
message(STATUS "[LOG/phxecho] SOURCES: ${SOURCES}")

add_executable(phxecho ${SOURCES})
add_dependencies(phxecho phxpaxos plugin)
target_link_libraries(
  phxecho
  ${CMAKE_BINARY_DIR}/src/libphxpaxos.a
  ${CMAKE_BINARY_DIR}/plugin/libplugin.a
  pthread
  glog
  protobuf
  leveldb)
