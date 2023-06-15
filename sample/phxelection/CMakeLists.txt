cmake_minimum_required(VERSION 3.10)

# specify project name
project(phxelection VERSION 1.0.0)
cmake_policy(SET CMP0048 NEW)

# specify the C++ standard & compile flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# print message
message(STATUS "[LOG/phxelection] PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

# specify sys include & libs
include_directories(BEFORE /usr/include /usr/local/include)
link_directories(BEFORE /usr/lib /usr/local/lib)

aux_source_directory(${PROJECT_SOURCE_DIR}/ SOURCES)
message(STATUS "[LOG/phxelection] SOURCES: ${SOURCES}")

add_executable(phxelection ${SOURCES})
target_include_directories(
  phxelection
  PRIVATE ${PROJECT_SOURCE_DIR}/../../include
  PRIVATE ${PROJECT_SOURCE_DIR}/../../plugin/include)
target_link_libraries(phxelection ${CMAKE_BINARY_DIR}/src/libphxpaxos.a
                      protobuf leveldb)
