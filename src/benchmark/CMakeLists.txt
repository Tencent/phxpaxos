# benchmark db
set(BENCHMARK_DB_SRC bench_db.cpp)
add_executable(benchmark_db ${BENCHMARK_DB_SRC})
target_include_directories(
  benchmark_db
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../logstorage
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../comm
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../utils
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../include)
target_link_libraries(benchmark_db ${CMAKE_BINARY_DIR}/src/libphxpaxos.a
                      protobuf leveldb)

# benchmark fsync
set(BENCHMARK_FSYNC_SRC fsync_bench.cpp)
add_executable(benchmark_fsync ${BENCHMARK_FSYNC_SRC})
target_include_directories(benchmark_fsync
                           PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../include)
target_link_libraries(benchmark_fsync ${CMAKE_BINARY_DIR}/src/libphxpaxos.a)

# benchmark main
set(BENCHMARK_MAIN_SRC bench_sm.cpp bench_server.cpp bench_main.cpp)
add_executable(benchmark_main ${BENCHMARK_MAIN_SRC})
target_include_directories(
  benchmark_main
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../utils
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../include)
target_link_libraries(benchmark_main ${CMAKE_BINARY_DIR}/src/libphxpaxos.a
                      protobuf leveldb)
