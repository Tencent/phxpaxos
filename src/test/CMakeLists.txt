file(GLOB PHX_PAXOS_TEST_SRCS ${CMAKE_CURRENT_SOURCE_DIR})
add_executable(phx_paxos_test ${PHX_PAXOS_TEST_SRCS})
target_link_libraries(
    phx_paxos_test
    PRIVATE
    phxpaxos::utils
    phxpaxos::comm
    phxpaxos
    leveldb
    protobuf::libprotobuf
    pthread
)
target_include_directories(
    phx_paxos_test
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)