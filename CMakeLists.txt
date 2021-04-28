cmake_minimum_required(VERSION 3.12)
project(phxpaxos)

option(PHXPAXOS_BUILD_TOOLS "build tools" OFF)
option(PHXPAXOS_BUILD_UT "build ut" OFF)
option(PHXPAXOS_BUILD_TEST "build test" OFF)
option(PHXPAXOS_BUILD_BENCHMARK "build benchmark" OFF)
option(PHXPAXOS_BUILD_PLUGIN "build plugin" OFF)
option(PHXPAXOS_BUILD_SAMPLE_ECHO "build sample phxecho" OFF)
option(PHXPAXOS_BUILD_SAMPLE_ELECTION "build sample phxelection" OFF)
option(PHXPAXOS_BUILD_SAMPLE_KV "build sample phxkv" OFF)
option(PHXPAXOS_INSTALL "create target to install phxpaxos library and headers" ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    # Release - Adds the -O3 -DNDEBUG flags to the compiler
    # Debug - Adds the -g flag
    # MinSizeRel - Adds -Os -DNDEBUG
    # RelWithDebInfo - Adds -O2 -g -DNDEBUG flags
    message("Setting build type to 'RelWithDebInfo' as none was specified.")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
        "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_compile_options(-Wall -Werror -Wno-unused-local-typedefs -Wno-unused-variable)
add_compile_options(-Wl,--no-as-needed)
add_compile_options(-std=c++11)

# for find_package in CONFIG mode to find <PackageName>Config.cmake and <lower-case-package-name>-config.cmake
list(APPEND CMAKE_PREFIX_PATH
    "${PROJECT_SOURCE_DIR}/third_party/local_install/abseil/lib64/cmake/absl"
    "${PROJECT_SOURCE_DIR}/third_party/local_install/grpc/lib/cmake/grpc"
)

# for find_library find_path find_program
list(APPEND CMAKE_PREFIX_PATH
    "${PROJECT_SOURCE_DIR}/third_party/local_install/gflags"
    "${PROJECT_SOURCE_DIR}/third_party/local_install/glog"
    "${PROJECT_SOURCE_DIR}/third_party/local_install/gmock"
    "${PROJECT_SOURCE_DIR}/third_party/local_install/leveldb"
    "${PROJECT_SOURCE_DIR}/third_party/local_install/protobuf"
    "${PROJECT_SOURCE_DIR}/third_party/local_install/grpc"
)

# for find_package in MODULE mode to find Find<PackageName>.cmake
list(INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/cmake)


find_package(Leveldb MODULE REQUIRED)
find_package(Protobuf MODULE REQUIRED)

add_subdirectory(src/algorithm)
add_subdirectory(src/checkpoint)
add_subdirectory(src/comm)
add_subdirectory(src/communicate)
add_subdirectory(src/config)
add_subdirectory(src/logstorage)
add_subdirectory(src/master)
add_subdirectory(src/node)
add_subdirectory(src/sm-base)
add_subdirectory(src/utils)

if(PHXPAXOS_BUILD_TOOLS)
    add_subdirectory(src/tools)
endif()

if(PHXPAXOS_BUILD_UT)
    find_package(Gtest MODULE REQUIRED)
    find_package(Gmock MODULE REQUIRED)
    add_subdirectory(src/ut)
endif()

if(PHXPAXOS_BUILD_TEST)
    add_subdirectory(src/test)
endif()

if(PHXPAXOS_BUILD_BENCHMARK)
    add_subdirectory(src/benchmark)
endif()

if(PHXPAXOS_BUILD_PLUGIN 
  OR PHXPAXOS_BUILD_SAMPLE_ECHO 
  OR PHXPAXOS_BUILD_SAMPLE_ELECTION 
  OR PHXPAXOS_BUILD_SAMPLE_KV)
    find_package(Gflags MODULE REQUIRED)
    find_package(Glog MODULE REQUIRED)
    add_subdirectory(plugin)
endif()

if(PHXPAXOS_BUILD_SAMPLE_ECHO)
    add_subdirectory(sample/phxecho)
endif()

if(PHXPAXOS_BUILD_SAMPLE_ELECTION)
    add_subdirectory(sample/phxelection)
endif()

if(PHXPAXOS_BUILD_SAMPLE_KV)
    find_package(absl CONFIG REQUIRED)
    find_package(gRPC CONFIG REQUIRED)
    include(${PROJECT_SOURCE_DIR}/cmake/gRPCHelper.cmake)
    add_subdirectory(sample/phxkv)
endif()
