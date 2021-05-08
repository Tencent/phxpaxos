set(PHXECHO_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/echo_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/echo_sm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
add_executable(phxecho ${PHXECHO_SRCS})
target_include_directories(
    phxecho
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    phxecho
    PRIVATE
    phxpaxos
    phxpaxos::plugin
)