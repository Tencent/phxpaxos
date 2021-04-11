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

message("GFLAGS_INCLUDE_DIR: ${GFLAGS_INCLUDE_DIR}")
message("GFLAGS_LIBRARY: ${GFLAGS_LIBRARY}")

add_library(gflags STATIC IMPORTRED)
set_target_properties(
    gflags
    INTERFACE_INCLUDE_DIRECTORIES ${GFLAGS_INCLUDE_DIR}
    IMPORTED_LOCATION ${GFLAGS_LIBRARY}
)