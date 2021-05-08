set(PHXPAXOS_NODE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/group.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pnode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/node.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/propose_batch.cpp
)
add_library(phxpaxos STATIC ${PHXPAXOS_NODE_SRCS})
target_link_libraries(phxpaxos PRIVATE -static-libstdc++)
target_link_libraries(
    phxpaxos
    PRIVATE
    phxpaxos::algorithm
    phxpaxos::comm
    phxpaxos::communicate
    phxpaxos::config
    phxpaxos::logstorage
    phxpaxos::master
)
target_include_directories(
    phxpaxos
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
if(PHXPAXOS_INSTALL)
    install(
        TARGETS phxpaxos
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
    install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/include/phxpaxos
        DESTINATION include
    )
endif()


set(TEST_PROPOSE_BATCH_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/test_propose_batch.cpp)
add_executable(test_propose_batch ${TEST_PROPOSE_BATCH_SRCS})
target_link_libraries(
    test_propose_batch
    PRIVATE
    phxpaxos::comm
    phxpaxos
)