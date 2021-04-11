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

message("LEVELDB_INCLUDE_DIR: ${LEVELDB_INCLUDE_DIR}")
message("LEVELDB_LIBRARY: ${LEVELDB_LIBRARY}")

add_library(levledb STATIC IMPORTRED)
set_target_properties(
    leveldb
    INTERFACE_INCLUDE_DIRECTORIES ${LEVELDB_INCLUDE_DIR}
    IMPORTED_LOCATION ${LEVELDB_LIBRARY}
)