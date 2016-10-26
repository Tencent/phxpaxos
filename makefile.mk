SRC_BASE_PATH ?= .
PREFIX?=$(SRC_BASE_PATH)
PHX_LIB_PATH = $(SRC_BASE_PATH)/.lib
PHX_SBIN_PATH = $(SRC_BASE_PATH)/.sbin
PHX_EXTLIB_PATH = $(PHX_LIB_PATH)/extlib

NANOPBPATH=$(SRC_BASE_PATH)/third_party/nanopb/

PROTOBUF_INCLUDE_PATH=$(SRC_BASE_PATH)/third_party/protobuf/include/
GRPC_INCLUDE_PATH=$(SRC_BASE_PATH)/third_party/grpc/include 
LEVELDB_INCLUDE_PATH=$(SRC_BASE_PATH)/third_party/leveldb/include/
GLOG_INCLUDE_PATH=$(SRC_BASE_PATH)/third_party/glog/src/
PHXPAXOS_INCLUDE_PATH=$(SRC_BASE_PATH)/include
PHXPAXOS_PLUGIN_PATH=$(SRC_BASE_PATH)/plugin/include

PROTOBUF_LIB_PATH=$(SRC_BASE_PATH)/third_party/protobuf/lib
LEVELDB_LIB_PATH=$(SRC_BASE_PATH)/third_party/leveldb/lib/
GLOG_LIB_PATH=$(SRC_BASE_PATH)/third_party/glog/lib
GRPC_LIBE_PATH=$(SRC_BASE_PATH)/third_party/grpc/lib
OPEN_SSL_LIB_PATH=$(SRC_BASE_PATH)/third_party/openssl/lib
PHXPAXOS_LIB_PATH=$(SRC_BASE_PATH)/lib

ifeq ($(debug),y)
# (1) Debug
	OPT = -g2
else
# (2) Production
	OPT = -O2
endif

CXX=g++
CXXFLAGS+=-std=c++11 $(OPT)
CPPFLAGS+=-I$(SRC_BASE_PATH) -I$(PROTOBUF_INCLUDE_PATH)  -I$(LEVELDB_INCLUDE_PATH)
CPPFLAGS+=-I$(GLOG_INCLUDE_PATH) 
CPPFLAGS+=-Wall -fPIC -m64  -Wno-unused-local-typedefs

#LDFLAGS+=-shared
#LDFLAGS+=-static 
LDFLAGS+=-L$(PHX_LIB_PATH) -L$(PROTOBUF_LIB_PATH) -L$(LEVELDB_LIB_PATH)
LDFLAGS+=-L$(GLOG_LIB_PATH) -L$(GRPC_LIBE_PATH) -L$(OPEN_SSL_LIB_PATH) -g
LDFLAGS+=-Wl,--no-as-needed


#=====================================================================================================

PROTOC = $(SRC_BASE_PATH)/third_party/protobuf/bin/protoc
PROTOS_PATH = .
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`
NANOPB_PLUGIN_PATH=$(NANOPBPATH)/generator/protoc-gen-nanopb

vpath %.proto $(PROTOS_PATH)

.PRECIOUS: %.grpc.pb.cc
%.grpc.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

.PRECIOUS: %.nano.pb.cc
%.nano.pb.cc: %.proto
	$(PROTOC) --plugin=protoc-gen-nanopb=$(NANOPB_PLUGIN_PATH) --nanopb_out=nanoproto/ $<


.PRECIOUS: %.pb.cc
%.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<


PROTOC_CHECK_VERSION_CMD = $(PROTOC) --version | grep -q libprotoc.3
HAS_VALID_PROTOC ?= $(shell $(PROTOC_CHECK_VERSION_CMD) 2> /dev/null && echo true || echo false)

LEVELDB_CHECK_CMD = ls $(LEVELDB_INCLUDE_PATH) > /dev/null
HAS_LEVEL_DB_CMD ?= $(shell $(LEVELDB_CHECK_CMD) 2> /dev/null && echo true || echo false)

.PHONY:verify-install
verify-install:
ifeq ($(HAS_VALID_PROTOC),true)
	@echo "protobuf check [done]"
else
	@echo "The library requires protobuf-compiler 3.0.0+"
	@echo "But the third party directory doesn't have it"
	@echo "Please check it and make sure the protobuf "
	@echo "has been placed or linked in the third party directory"
endif

ifeq ($(HAS_LEVEL_DB_CMD),true)
	@echo "level check [done]"
else
	@echo "The library requires the leveldb library"
	@echo "But the third party directory doesn't have it"
	@echo "Please check it and make sure the leveldb"
	@echo "has been placed or linked in the third party directory"
endif

.PHONY:install
install:
	@prefix_dir=`readlink $(PREFIX) -m`;\
	src_dir=`readlink $(SRC_BASE_PATH) -m`;\
	if ([ "$$prefix_dir" != "$$src_dir" ]); then \
	echo cp $(PHX_LIB_PATH) $(PREFIX)/include -rf;\
	cp $(PHXPAXOS_INCLUDE_PATH) $(PREFIX)/include -rf;\
	fi
	echo INSTALL to $(PREFIX)/lib;
	@mkdir $(PREFIX)/lib -p;\
	rm $(PREFIX)/lib/* -rf;\
	cp $(PHX_EXTLIB_PATH)/* $(PREFIX)/lib/ -rf;

version = 1.0.0

dist: clean phxpaxos-$(version).src.tgz
phxpaxos-$(version).src.tgz:
	@rm -rf phxpaxos-$(version).src.tgz
	@find ./* -name "Makefile" | xargs rm -rf
	@find . -type f | grep -v CVS | grep -v "lib.*.a" | grep -v "license\.py" | grep -v .svn | grep -v .git | sed s:^./:phxpaxos-$(version)/: > MANIFEST
	@(cd ..; ln -s phxpaxos phxpaxos-$(version))
	(cd ..; tar cvf - `cat phxpaxos/MANIFEST` | gzip > phxpaxos/phxpaxos-$(version).src.tgz)
	@(cd ..; rm phxpaxos-$(version))

