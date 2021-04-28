#!/bin/bash

set -e -u -o pipefail

work_dir=$(cd `dirname $0` && pwd)
mkdir -p ${work_dir}/local_install/{gflags,glog,gmock,leveldb,abseil,protobuf,grpc}
install_prefix=${work_dir}/local_install

function perror() {
    echo -e "\033[0;31;1m$1\033[0m"
}

function psucc() {
    echo -e "\e[1;32m$1\e[0m"
}

# gflags
function install_gflags() {
    cd ${work_dir}/gflags
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=${install_prefix}/gflags \
        .
    make
    make install
    psucc "install gflags ok."
}

# glog
function install_glog() {
    cd ${work_dir}/glog
    mkdir -p build && cd build
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=${install_prefix}/glog \
        ..
    make
    make install
    psucc "install glog ok."
}

# gmock
function install_gmock_and_gtest() {
    cd ${work_dir}/googletest
    mkdir -p build && cd build
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=${install_prefix}/gmock \
        -DBUILD_GMOCK=ON \
        ..
    make
    make install
    psucc "install gmock & gtest ok."
}

# leveldb
function install_leveldb() {
    cd ${work_dir}/leveldb
    # enable rtti
    sed -i '/-fno-rtti/d' CMakeLists.txt
    sed -i '/-frtti/d' CMakeLists.txt
    mkdir -p build && cd build
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DLEVELDB_BUILD_TESTS=OFF \
        -DLEVELDB_BUILD_BENCHMARKS=OFF \
        -DCMAKE_INSTALL_PREFIX=${install_prefix}/leveldb \
        ..
    cmake --build .
    make install
    psucc "install leveldb ok."
}

# abseil-cpp
function install_abseil() {
    cd ${work_dir}/grpc/third_party/abseil-cpp
    mkdir -p build && cd build
    cmake \
        -DCMAKE_INSTALL_PREFIX=${install_prefix}/abseil \
        ..
    make
    make install
    psucc "install abseil-cpp ok."
}

# protobuf
function install_protobuf() {
    cd ${work_dir}/grpc/third_party/protobuf
    ./autogen.sh
    ./configure --prefix=${install_prefix}/protobuf --disable-shared
    make
    make install
    psucc "install protobuf ok."
}

# grpc
function install_grpc() {
    cd ${work_dir}/grpc
    mkdir -p cmake/build && cd cmake/build
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=${install_prefix}/grpc \
        ../..
    make
    make install
    psucc "install grpc ok."
}

install_gflags
install_glog
install_gmock_and_gtest
install_leveldb
install_abseil
install_protobuf
install_grpc

psucc "all done."
