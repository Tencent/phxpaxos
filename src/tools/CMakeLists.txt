add_executable(system_variables_tools system_variables_tools.cpp)
target_link_libraries(
    system_variables_tools
    PRIVATE
    phxpaxos::comm
    phxpaxos::config
    phxpaxos::logstorage
)
target_compile_options(system_variables_tools PRIVATE -Wno-format)

add_executable(paxos_log_tools paxos_log_tools.cpp)
target_link_libraries(
    paxos_log_tools
    PRIVATE
    phxpaxos::comm
    phxpaxos::logstorage
)
target_compile_options(paxos_log_tools PRIVATE -Wno-format)

add_executable(vfile_fetch vfile_fetch.cpp)
target_link_libraries(
    vfile_fetch
    PRIVATE
    phxpaxos::comm
    phxpaxos::logstorage
)
target_compile_options(vfile_fetch PRIVATE -Wno-format)
