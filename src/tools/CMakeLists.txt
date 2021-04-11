add_executable(system_variables_tools system_variables_tools.cpp)
target_link_libraries(
    system_variables_tools
    PRIVATE
    phxpaxos::comm
    phxpaxos::config
    phxpaxos::logstorage
)

add_executable(paxos_log_tools paxos_log_tools.cpp)
target_link_libraries(
    paxos_log_tools
    PRIVATE
    phxpaxos::comm
    phxpaxos::logstorage
)

add_executable(vfile_fetch vfile_fetch.cpp)
target_link_libraries(
    vfile_fetch
    PRIVATE
    phxpaxos::comm
    phxpaxos::logstorage
)