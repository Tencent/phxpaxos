# test1
aux_source_directory(. TEST_SOURCES)
message(STATUS "[LOG/test1] TEST_SOURCES: ${TEST_SOURCES}")
add_executable(test1 ${TEST_SOURCES})
add_dependencies(test1 phxpaxos)
target_link_libraries(test1 ${CMAKE_BINARY_DIR}/src/libphxpaxos.a pthread
                      protobuf leveldb)
