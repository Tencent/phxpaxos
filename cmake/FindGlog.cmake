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

add_library(glog STATIC IMPORTED)
set_target_properties(glog PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GLOG_INCLUDE_DIR}
    IMPORTED_LOCATION ${GLOG_LIBRARY}
)

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(glog DEFAULT_MSG
    GLOG_LIBRARY GLOG_INCLUDE_DIR)