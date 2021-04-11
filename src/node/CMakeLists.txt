set(PHXPAXOS_NODE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/group.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pnode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/node.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/propose_batch.cpp
)
add_library(phxpaxos_node STATIC ${PHXPAXOS_NODE_SRCS})
add_library(phxpaxos::node ALIAS phxpaxos_node)
target_link_libraries(
    phxpaxos_node
    PRIVATE
    phxpaxos::comm
    phxpaxos::logstorage
    phxpaxos::communicate
    phxpaxos::algorithm
    phxpaxos::config
    phxpaxos::master
)
target_include_directories(
    phxpaxos_node
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(TEST_PROPOSE_BATCH_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/test_propose_batch.cpp)
add_executable(test_propose_batch ${TEST_PROPOSE_BATCH_SRCS})
target_link_libraries(
    test_propose_batch
    phxpaxos::node
)