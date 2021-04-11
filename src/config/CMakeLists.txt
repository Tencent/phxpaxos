file(GLOB PHXPAXOS_CONFIG_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(phxpaxos_config STATIC ${PHXPAXOS_CONFIG_SRCS})
add_library(phxpaxos::config ALIAS phxpaxos_config)
target_link_libraries(
    phxpaxos_config
    PRIVATE
    phxpaxos::comm
    phxpaxos::logstorage
    phxpaxos::smbase
)
target_include_directories(
    phxpaxos_config
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
