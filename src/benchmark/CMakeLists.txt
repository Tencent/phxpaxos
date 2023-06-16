# benchmark db
set(BENCHMARK_DB_SRC bench_db.cpp)
add_executable(benchmark_db ${BENCHMARK_DB_SRC})
add_dependencies(benchmark_db phxpaxos)
target_link_libraries(benchmark_db ${CMAKE_BINARY_DIR}/src/libphxpaxos.a
                      pthread protobuf leveldb)

# benchmark fsync
set(BENCHMARK_FSYNC_SRC fsync_bench.cpp)
add_executable(benchmark_fsync ${BENCHMARK_FSYNC_SRC})

# benchmark main
set(BENCHMARK_MAIN_SRC bench_sm.cpp bench_server.cpp bench_main.cpp)
add_executable(benchmark_main ${BENCHMARK_MAIN_SRC})
add_dependencies(benchmark_main phxpaxos)
target_link_libraries(benchmark_main ${CMAKE_BINARY_DIR}/src/libphxpaxos.a
                      pthread protobuf leveldb)
