file(GLOB PHXPAXOS_COMMUNICATE_TCP_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(phxpaxos_communicate_tcp STATIC ${PHXPAXOS_COMMUNICATE_TCP_SRCS})
add_library(phxpaxos::communicate::tcp ALIAS phxpaxos_communicate_tcp)
target_link_libraries(
    phxpaxos_communicate_tcp
    PRIVATE
    phxpaxos::utils
    phxpaxos::comm
)
target_include_directories(
    phxpaxos_communicate_tcp
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)