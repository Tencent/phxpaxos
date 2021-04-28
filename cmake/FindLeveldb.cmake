find_path(LEVELDB_INCLUDE_DIR
    NAMES leveldb/db.h
    PATHS "${CMAKE_PREFIX_PATH}/include"
)
mark_as_advanced(LEVELDB_INCLUDE_DIR)

find_library(LEVELDB_LIBRARY
    NAMES leveldb
    PATHS "${CMAKE_PREFIX_PATH}/lib" "${CMAKE_PREFIX_PATH}/lib64"
)
mark_as_advanced(LEVELDB_LIBRARY)

add_library(leveldb STATIC IMPORTED)
set_target_properties(leveldb PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${LEVELDB_INCLUDE_DIR}
    IMPORTED_LOCATION ${LEVELDB_LIBRARY}
)

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(leveldb DEFAULT_MSG
    LEVELDB_LIBRARY LEVELDB_INCLUDE_DIR)