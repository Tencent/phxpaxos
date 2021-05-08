file(GLOB PHXPAXOS_SMBASE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(phxpaxos_smbase STATIC ${PHXPAXOS_SMBASE_SRCS})
add_library(phxpaxos::smbase ALIAS phxpaxos_smbase)
target_link_libraries(
    phxpaxos_smbase
    PUBLIC
    phxpaxos::comm
)
target_include_directories(
    phxpaxos_smbase
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)