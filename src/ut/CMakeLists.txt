# ut
aux_source_directory("${CMAKE_CURRENT_LIST_DIR}" UT_SOURCES)
message(STATUS "[LOG/ut] UT_SOURCES: ${UT_SOURCES}")
add_executable(ut ${UT_SOURCES})
target_include_directories(
  ut
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../include
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../algorithm
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../comm
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../utils
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../config
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../logstorage
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../communicate
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../sm-base
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../checkpoint)

target_link_libraries(
  ut
  ${CMAKE_BINARY_DIR}/src/libphxpaxos.a
  protobuf
  gtest
  gmock
  glog
  gflags
  leveldb)
