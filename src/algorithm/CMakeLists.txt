file(GLOB PHXPAXOS_ALGORITHM_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(phxpaxos_algorithm STATIC ${PHXPAXOS_ALGORITHM_SRCS})
add_library(phxpaxos::algorithm ALIAS phxpaxos_algorithm)
target_link_libraries(
    phxpaxos_algorithm
    PRIVATE
    phxpaxos::comm
    phxpaxos::logstorage
    phxpaxos::smbase
    phxpaxos::checkpoint
    phxpaxos::config
)
target_include_directories(
    phxpaxos_algorithm
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
)