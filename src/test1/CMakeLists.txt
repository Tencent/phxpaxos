# test1
aux_source_directory("${CMAKE_CURRENT_LIST_DIR}" TEST_SOURCES)
message(STATUS "[LOG/test1] TEST_SOURCES: ${TEST_SOURCES}")
add_executable(test1 ${TEST_SOURCES})
target_include_directories(
  test1
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../comm
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../utils)
target_link_libraries(test1 ${CMAKE_BINARY_DIR}/src/libphxpaxos.a protobuf
                      leveldb)
