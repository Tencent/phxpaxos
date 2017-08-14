#!/bin/bash

pushd third_party/glog
CFLAGS="-m64" CXXFLAGS="-m64" ./configure --prefix=$(pwd)
if [ $? -ne 0 ]; then
	echo "failed to configure glog"
	return
fi
popd

python third_party/gyp/gyp_main.py --depth=. --include=common.gypi -Dtarget_arch=x64 --generator-output=. -f ninja phxpaxos.gyp
if [ $? -ne 0 ]; then
	echo "failed to run gyp"
	return
fi
ninja -C out/Release