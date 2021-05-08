set(PHXPAXOS_PLUGIN_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/logger_google/logger_google.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logger_google/logger_google_impl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/monitor/monitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/monitor/monitor_bp.cpp
)
add_library(phxpaxos_plugin STATIC ${PHXPAXOS_PLUGIN_SRCS})
add_library(phxpaxos::plugin ALIAS phxpaxos_plugin)
target_link_libraries(
    phxpaxos_plugin
    PRIVATE
    phxpaxos
    glog
    gflags
)
target_include_directories(
    phxpaxos_plugin
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/logger_google
    ${CMAKE_CURRENT_SOURCE_DIR}/monitor
)