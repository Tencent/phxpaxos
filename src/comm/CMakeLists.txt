set(PHXPAXOS_COMM_PROTOS ${CMAKE_CURRENT_SOURCE_DIR}/paxos_msg.proto)
protobuf_generate_cpp(PHXPAXOS_COMM_PROTO_SRCS PHXPAXOS_COMM_PROTO_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR} ${PHXPAXOS_COMM_PROTOS})

file(GLOB PHXPAXOS_COMM_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(phxpaxos_comm STATIC
    ${PHXPAXOS_COMM_SRCS}
    ${PHXPAXOS_COMM_PROTO_SRCS}
)
add_library(phxpaxos::comm ALIAS phxpaxos_comm)
target_link_libraries(
    phxpaxos_comm
    PUBLIC
    phxpaxos::utils
    protobuf::libprotobuf
)
target_include_directories(
    phxpaxos_comm
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
)