file(GLOB PHXPAXOS_UTILS_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(phxpaxos::utils STATIC ${PHXPAXOS_UTILS_SRCS})
target_link_libraries(
    phxpaxos_utils
    PRIVATE
    pthread
)
target_include_directories(
    phxpaxos_utils
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)