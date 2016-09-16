
set -e  # exit immediately on error
set -x  # display all commands

cd third_party;

if [ ! -f protobuf/bin/protoc ]; then
	if [ ! -f protobuf-cpp-3.0.0.tar.gz ]; then
		wget https://github.com/google/protobuf/releases/download/v3.0.0/protobuf-cpp-3.0.0.tar.gz
	fi	

	tar zxvf protobuf-cpp-3.0.0.tar.gz
	cd protobuf-3.0.0

	./configure --prefix=`pwd`/../protobuf
	make -j2
	make install

	cd ../
fi

if [ ! -f leveldb/lib/libleveldb.a ]; then
	if [ ! -f v1.19.tar.gz ]; then
		wget https://github.com/google/leveldb/archive/v1.19.tar.gz
	fi

	rm -rf leveldb

	tar zxvf v1.19.tar.gz
	ln -s leveldb-1.19 leveldb
	cd leveldb

	make

	mkdir -p lib
	cp out-static/libleveldb.a lib/

	cd ../
fi

cd ..

./autoinstall.sh

make

