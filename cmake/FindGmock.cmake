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

message("GMOCK_INCLUDE_DIR: ${GMOCK_INCLUDE_DIR}")
message("GMOCK_LIBRARY: ${GMOCK_LIBRARY}")

add_library(gmock STATIC IMPORTRED)
set_target_properties(
    gmock
    INTERFACE_INCLUDE_DIRECTORIES ${GMOCK_INCLUDE_DIR}
    IMPORTED_LOCATION ${GMOCK_LIBRARY}
)