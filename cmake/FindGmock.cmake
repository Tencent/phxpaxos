find_path(GMOCK_INCLUDE_DIR
    NAMES gmock/gmock.h
    PATHS "${CMAKE_PREFIX_PATH}/include"
)
mark_as_advanced(GMOCK_INCLUDE_DIR)

find_library(GMOCK_LIBRARY
    NAMES gmock
    PATHS "${CMAKE_PREFIX_PATH}/lib" "${CMAKE_PREFIX_PATH}/lib64"
)
mark_as_advanced(GMOCK_LIBRARY)

add_library(gmock STATIC IMPORTED)
set_target_properties(gmock PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GMOCK_INCLUDE_DIR}
    IMPORTED_LOCATION ${GMOCK_LIBRARY}
)

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(gmock DEFAULT_MSG
    GMOCK_LIBRARY GMOCK_INCLUDE_DIR)