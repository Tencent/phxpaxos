find_path(GFLAGS_INCLUDE_DIR
    NAMES gflags/gflags.h
    PATHS "${CMAKE_PREFIX_PATH}/include"
)
mark_as_advanced(GFLAGS_INCLUDE_DIR)

find_library(GFLAGS_LIBRARY
    NAMES gflags
    PATHS "${CMAKE_PREFIX_PATH}/lib" "${CMAKE_PREFIX_PATH}/lib64"
)
mark_as_advanced(GFLAGS_LIBRARY)

add_library(gflags STATIC IMPORTED)
set_target_properties(gflags PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GFLAGS_INCLUDE_DIR}
    IMPORTED_LOCATION ${GFLAGS_LIBRARY}
)

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(gflags DEFAULT_MSG
    GFLAGS_LIBRARY GFLAGS_INCLUDE_DIR)