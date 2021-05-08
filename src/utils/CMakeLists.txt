file(GLOB PHXPAXOS_UTILS_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(phxpaxos_utils STATIC ${PHXPAXOS_UTILS_SRCS})
add_library(phxpaxos::utils ALIAS phxpaxos_utils)
target_link_libraries(
    phxpaxos_utils
    PUBLIC
    pthread
)
target_include_directories(
    phxpaxos_utils
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)