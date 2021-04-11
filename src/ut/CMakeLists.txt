file(GLOB PHXPAXOS_UT_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(phxpaxos_ut ${PHXPAXOS_UT_SRCS})
target_link_libraries(
    phxpaxos_ut
    PRIVATE
    phxpaxos::logstorage
    phxpaxos::config
    phxpaxos::algorithm
    phxpaxos::communicate
    gmock
    gtest
)
target_include_directories(
    phxpaxos_ut
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)