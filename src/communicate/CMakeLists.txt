add_subdirectory(tcp)

file(GLOB PHXPAXOS_COMMUNICATE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(phxpaxos_communicate STATIC ${PHXPAXOS_COMMUNICATE_SRCS})
add_library(phxpaxos::communicate ALIAS phxpaxos_communicate)
target_link_libraries(
    phxpaxos_communicate
    PRIVATE
    phxpaxos::comm
    phxpaxos::utils
    phxpaxos::config
    phxpaxos::communicate::tcp
)
target_include_directories(
    phxpaxos_communicate
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)