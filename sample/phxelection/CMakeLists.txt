cmake_minimum_required(VERSION 3.10)

project(phxelection VERSION 1.0.0)
cmake_policy(SET CMP0048 NEW)

# print message
message(STATUS "[LOG/phxelection] PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

aux_source_directory(. SOURCES)
message(STATUS "[LOG/phxelection] SOURCES: ${SOURCES}")

add_executable(phxelection ${SOURCES})
add_dependencies(phxelection phxpaxos)
target_link_libraries(phxelection ${CMAKE_BINARY_DIR}/src/libphxpaxos.a pthread
                      protobuf leveldb)
