find_path(GLOG_INCLUDE_DIR
    NAMES glog/logging.h
    PATHS "${CMAKE_PREFIX_PATH}/include"
)
mark_as_advanced(GLOG_INCLUDE_DIR)

find_library(GLOG_LIBRARY
    NAMES glog
    PATHS "${CMAKE_PREFIX_PATH}/lib" "${CMAKE_PREFIX_PATH}/lib64"
)
mark_as_advanced(GLOG_LIBRARY)

message("GLOG_INCLUDE_DIR: ${GLOG_INCLUDE_DIR}")
message("GLOG_LIBRARY: ${GLOG_LIBRARY}")

add_library(glog STATIC IMPORTRED)
set_target_properties(
    glog
    INTERFACE_INCLUDE_DIRECTORIES ${GLOG_INCLUDE_DIR}
    IMPORTED_LOCATION ${GLOG_LIBRARY}
)