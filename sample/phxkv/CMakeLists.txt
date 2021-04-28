set(PHXKV_PROTOS ${CMAKE_CURRENT_SOURCE_DIR}/phxkv.proto)
protobuf_generate_cpp(PHXKV_PROTO_SRCS PHXKV_PROTO_HDRS ${CMAKE_CURRENT_SOURCE_DIR} ${PHXKV_PROTOS})
grpc_generate_cpp(PHXKV_GRPC_SRCS PHXKV_GRPC_HDRS ${CMAKE_CURRENT_SOURCE_DIR} ${PHXKV_PROTOS})

set(PHXKV_CLIENT_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/kv_grpc_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kv_grpc_client_main.cpp
)
add_library(phxkv_client STATIC 
    ${PHXKV_CLIENT_SRCS}
    ${PHXKV_PROTO_SRCS}
    ${PHXKV_GRPC_SRCS}
)
target_include_directories(
    phxkv_client
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    phxkv_client
    PUBLIC
    phxpaxos
    gRPC::grpc
    gRPC::grpc++_unsecure
    absl::synchronization
)
target_compile_options(phxkv_client PRIVATE -Wall)

set(PHXKV_CLIENT_TOOLS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/kv_grpc_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kv_grpc_client_main.cpp
)
add_executable(phxkv_client_tools
    ${PHXKV_CLIENT_TOOLS_SRCS}
    ${PHXKV_PROTO_SRCS}
    ${PHXKV_GRPC_SRCS}
)
target_include_directories(
    phxkv_client_tools
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    phxkv_client_tools
    PRIVATE
    phxpaxos
    gRPC::grpc
    gRPC::grpc++_unsecure
    absl::synchronization
)
target_compile_options(phxkv_client_tools PRIVATE -Wall)

set(PHXKV_GRPCSERVER_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/kv.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kvsm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kv_paxos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/log.cpp  
    ${CMAKE_CURRENT_SOURCE_DIR}/kv_grpc_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kv_grpc_server_main.cpp
)
add_executable(phxkv_grpcserver
    ${PHXKV_GRPCSERVER_SRCS}
    ${PHXKV_PROTO_SRCS}
    ${PHXKV_GRPC_SRCS}
)
target_include_directories(
    phxkv_grpcserver
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    phxkv_grpcserver
    PRIVATE
    phxpaxos
    phxpaxos::plugin
    leveldb
    gRPC::grpc
    gRPC::grpc++_unsecure
    absl::synchronization
)
target_compile_options(phxkv_grpcserver PRIVATE -Wall)
