
import os
from build_comm import *

base_dir=""
lib_dir=""
bin_dir=""

third_party_list=["PROTOBUF", "LEVELDB"]

def GetPath(key):
	makefile_def=open("makefile.mk")
	lines = makefile_def.readlines()
	makefile_def.close()

	for line in lines:
		key_value = line.split('=')
		if( str.strip(key_value[0]) == key ):
			res=str.strip(key_value[1])
			res=res.replace("$(SRC_BASE_PATH)",base_dir);
			res=res.replace("$(PHX_LIB_PATH)",lib_dir);
			return res
	return ""

def GetPathPrefix(key):
	makefile_def=open("makefile.mk")
	lines = makefile_def.readlines()
	makefile_def.close()

	res_list=[]
	for line in lines:
		key_value = line.split('=')
		if( str.strip(key_value[0])[0:len(key)] == key and str.strip(key_value[0])[-4:]=="PATH"):
			res=str.strip(key_value[1])
			res=res.replace("$(SRC_BASE_PATH)",base_dir);
			res=res.replace("$(PHX_LIB_PATH)",lib_dir);
			res_list.append( (str.strip(key_value[0]),res) )
	return res_list

def CheckBasePath():
	global lib_dir, sbin_dir
	lib_dir=GetPath("PHX_LIB_PATH")

	if( not os.path.exists( lib_dir ) ):
		os.mkdir( lib_dir )

	sbin_dir=GetPath("PHX_SBIN_PATH")

	if( not os.path.exists( sbin_dir ) ):
		os.mkdir( sbin_dir )

	extlib_dir=GetPath("PHX_EXTLIB_PATH")
	if( not os.path.exists( extlib_dir ) ):
		os.mkdir( extlib_dir )

def Check3rdPath():

	for lib in third_party_list:
		path_list=GetPathPrefix(lib)
		for path in path_list:
			if( not os.path.exists( path[1] ) ):
				print("%s not found" % path[1])
				print("please make sure %s has been placed on third party directory" % lib)
				exit(1)

if(__name__ == '__main__'):
	base_dir=sys.argv[1]
	CheckBasePath()
	Check3rdPath()
	exit(0)
