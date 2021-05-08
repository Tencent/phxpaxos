find_path(GTEST_INCLUDE_DIR
    NAMES gtest/gtest.h
    PATHS "${CMAKE_PREFIX_PATH}/include"
)
mark_as_advanced(GTEST_INCLUDE_DIR)

find_library(GTEST_LIBRARY
    NAMES gtest
    PATHS "${CMAKE_PREFIX_PATH}/lib" "${CMAKE_PREFIX_PATH}/lib64"
)
mark_as_advanced(GTEST_LIBRARY)

add_library(gtest STATIC IMPORTED)
set_target_properties(gtest PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIR}
    IMPORTED_LOCATION ${GTEST_LIBRARY}
)

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(gtest DEFAULT_MSG
    GTEST_LIBRARY GTEST_INCLUDE_DIR)