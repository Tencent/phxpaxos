set(PHX_PAXOS_BENCH_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/bench_sm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bench_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bench_main.cpp
)
add_executable(phx_paxos_bench ${PHX_PAXOS_BENCH_SRCS})
target_link_libraries(
    phx_paxos_bench
    PRIVATE
    phxpaxos::utils
    phxpaxos::algorithm
    phxpaxos
    leveldb
    protobuf::libprotobuf
    pthread
)
target_include_directories(
    phx_paxos_bench
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
)

set(BENCH_DB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/bench_db.cpp)
add_executable(bench_db ${BENCH_DB_SRCS})
target_link_libraries(
    bench_db
    PRIVATE
    phxpaxos
    phxpaxos::logstorage
    leveldb
    protobuf::libprotobuf
    pthread
)
target_include_directories(
    bench_db
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
)