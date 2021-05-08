set(PHXELECTION_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/election.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/election_main.cpp
)
add_executable(phxelection ${PHXELECTION_SRCS})
target_include_directories(
    phxelection
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    phxelection
    PRIVATE
    phxpaxos
    phxpaxos::plugin
)