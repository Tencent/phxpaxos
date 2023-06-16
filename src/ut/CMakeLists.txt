# ut
aux_source_directory(. UT_SOURCES)
message(STATUS "[LOG/ut] UT_SOURCES: ${UT_SOURCES}")

add_executable(ut ${UT_SOURCES})
add_dependencies(ut phxpaxos)
target_link_libraries(
  ut
  ${CMAKE_BINARY_DIR}/src/libphxpaxos.a
  pthread
  gtest
  gmock
  protobuf
  leveldb)
