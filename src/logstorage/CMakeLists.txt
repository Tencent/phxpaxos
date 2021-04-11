file(GLOB PHXPAXOS_LOGSTORAGE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(phxpaxos_logstorage STATIC ${PHXPAXOS_LOGSTORAGE_SRCS})
add_library(phxpaxos::logstorage ALIAS phxpaxos_logstorage)
target_link_libraries(
    phxpaxos_logstorage
    PRIVATE
    phxpaxos::comm
    leveldb
)
target_include_directories(
    phxpaxos_logstorage
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)