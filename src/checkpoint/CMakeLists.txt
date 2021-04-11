file(GLOB PHXPAXOS_CHECKPOINT_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(phxpaxos_checkpoint STATIC ${PHXPAXOS_CHECKPOINT_SRCS})
add_library(phxpaxos::checkpoint ALIAS phxpaxos_checkpoint)
target_link_libraries(
    phxpaxos_checkpoint
    PRIVATE
    phxpaxos::comm
    phxpaxos::logstorage
    phxpaxos::smbase
    phxpaxos::utils
    phxpaxos::config
)
target_include_directories(
    phxpaxos_checkpoint
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)