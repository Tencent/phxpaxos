set(PHXPAXOS_MASTER_PROTOS ${CMAKE_CURRENT_SOURCE_DIR}/master_sm.proto)
protobuf_generate_cpp(PHXPAXOS_MASTER_PROTO_SRCS PHXPAXOS_MASTER_PROTO_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR} ${PHXPAXOS_MASTER_PROTOS})

file(GLOB PHXPAXOS_MASTER_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(phxpaxos_master STATIC
    ${PHXPAXOS_MASTER_SRCS}
    ${PHXPAXOS_MASTER_PROTO_SRCS}
)
add_library(phxpaxos::master ALIAS phxpaxos_master)
target_link_libraries(
    phxpaxos_master
    PUBLIC
    phxpaxos::utils
    phxpaxos::comm
    phxpaxos::config
    phxpaxos::logstorage
)
target_include_directories(
    phxpaxos_master
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)