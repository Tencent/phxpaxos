import sys
import os
import pwd
import getpass

from build_comm import *

base_path=""

include_makefile=[]
makefile=[]
obj_list=[]
clean_dir=[]

lib_path="$(SRC_BASE_PATH)/.lib"
ext_lib_path="$(SRC_BASE_PATH)/.lib/extlib"
sbin_path="$(SRC_BASE_PATH)/.sbin"

def writefile(write_file, text):
	write_file.append(text + "\n")

def Uniq(u_list, check_uniq=0):
	if( check_uniq == 0 ):
		for item in u_list:
			while(u_list.count(item) > 1):
				u_list.remove(item)
	else:
		res_list = []
		for item in u_list:
			if( res_list.count(item) == 0 ):
				res_list.append( item )
		u_list = res_list
	return u_list

res_list={}
makefile=None
makefile_name = "Makefile"

def GetSourceTagFromDeps(path, lib_name, tag_name, check_uniq=0):
	define_name = GetLableName(lib_name, tag_name)

	lib_define_name = GetLableName(lib_name, "LIB")
	res = []

	if(path.find("third_party") >=0):
		return res

	if((path,lib_name,tag_name) in res_list):
		return res_list[ (path,lib_name,tag_name) ]


	lib_path = base_path + "/" + path + "/" + include_makefile_name
	makefile_file = open(lib_path, "r")
	try:
		lines = makefile_file.readlines()
		for line in lines:
			values=line.split('=')
			find_local_res = []
			find_global_res = []
			if(values[0] == define_name):
				value = values[1].replace('\n', '').split(' ')
				for obj in value:
					if(len(obj) > 0):
						if(tag_name == "OBJ"):
							find_local_res.append("%s/%s" % (path, obj))
						elif(tag_name == "LIB"):
							if(len(obj.split(':')) == 1):
								find_local_res.append(obj)
						else:
							find_local_res.append(obj)
							
			if(values[0] == lib_define_name):
				res_inc = values[1].replace('\n','')
				dep_lib_list = res_inc.split(' ')
				for dep_lib in dep_lib_list:
					if(len(dep_lib.split(':')) > 1):
						deps_path = dep_lib.split(':')[0]
						deps_lib_name = dep_lib.split(':')[1]
						if(deps_path == ""):
							deps_path = path
						find_global_res+=GetSourceTagFromDeps(deps_path, deps_lib_name, tag_name, check_uniq)
			if( check_uniq == 0 ):
				res+= find_local_res
				res+= find_global_res
			else:
				res+= find_global_res
				res+= find_local_res
	finally:
		makefile_file.close()

	if( tag_name == "FULL_LIB_DEPS_PATH" ):
		res.append( "$(SRC_BASE_PATH)/%s" % path )

	res=Uniq(res, check_uniq)
	res_list[ (path,lib_name,tag_name) ] = res
	return res	


def PrintComm(path, target_name, lib_name):
	inc_res=GetSourceTagFromDeps(path, lib_name, "INCS")
	cppflags_res=GetSourceTagFromDeps(path, lib_name, "EXTRA_CPPFLAGS")
	full_lib_path_res=GetSourceTagFromDeps(path, lib_name, "FULL_LIB_DEPS_PATH", 1)
	for path in full_lib_path_res:
		if(path.find("third_party")!=-1):
			full_lib_path_res.remove(path)

	obj_name = "%s_%s" % (lib_name.upper(), "SRC")
	inc_name = "%s_%s" % (lib_name.upper(), "INCS")
	full_lib_path_name = "%s_%s" % (lib_name.upper(), "FULL_LIB_PATH")
	extra_cpp_flag_name = "%s_%s" % (lib_name.upper(), "EXTRA_CPPFLAGS")

	makefile.write("%s=$(%s)\n" % (obj_name, GetLableName(lib_name, "OBJ")))
	makefile.write("%s=$(sort %s)\n" % (inc_name, ' '.join(inc_res)))
	makefile.write("%s=%s\n" % (full_lib_path_name, ' '.join(full_lib_path_res)))
	makefile.write("%s=%s\n\n" % (extra_cpp_flag_name,' '.join(cppflags_res)))

	makefile.write("CPPFLAGS+=$(patsubst %%,-I%%, $(%s))\n" % inc_name)
	makefile.write("CPPFLAGS+=$(%s)\n\n" % extra_cpp_flag_name)

	return (obj_name,inc_name, full_lib_path_name, extra_cpp_flag_name)

def PrintReferenceDIR(target_name, direct_inc_name):
	makefile.write("%s_dir:$(%s)\n" % (target_name, direct_inc_name))
	makefile.write("\t@for dir in $^;\\\n")
	makefile.write("\tdo \\\n")
	makefile.write("\tcurrent_dir=`readlink $$dir -m`;\\\n");
	makefile.write("\tpwd_dir=`pwd`;\\\n");
	makefile.write("\tpwd_dir=`readlink $$pwd_dir -m`;\\\n");
	makefile.write("\tif ([ \"$$current_dir\" != \"$$pwd_dir\" ]); then \\\n");
	makefile.write("\tmake -C $$dir;\\\n");
	makefile.write("\tfi;\\\n");
	makefile.write("\tdone\n\n");


def PrintLib(path, target_name, lib_name, export = False):
	(obj_name,inc_name, direct_inc_name, extra_cpp_flag_name) = PrintComm(path, target_name, lib_name)

	if(export == True):
		makefile.write("lib_%s: %s_dir %s/lib%s.a %s/lib%s.a\n\n" % (target_name, target_name, lib_path, target_name, ext_lib_path, target_name))
		clean_dir.append( "%s/lib%s.a" % (lib_path, target_name))
		clean_dir.append( "%s/lib%s.a" % (ext_lib_path, target_name))

		PrintReferenceDIR(target_name, direct_inc_name)

		src_name = GetLableName(lib_name, "LIB_OBJ")	

		makefile.write("%s/lib%s.a: $(%s)\n" % (lib_path, target_name, obj_name))
		makefile.write("\tar -cvq $@ $(%s)\n\n" % (obj_name))

		src_res=GetSourceTagFromDeps(path, lib_name, "OBJ")
		makefile.write("%s=$(patsubst %%, $(SRC_BASE_PATH)/%%, %s)\n" % (src_name, ' '.join(src_res)))
		makefile.write("%s/lib%s.a: $(%s)\n" % (ext_lib_path, target_name, src_name))
		makefile.write("\tar -cvq $@ $(%s)\n\n" % (src_name))

	else:
		makefile.write("lib_%s:%s/lib%s.a\n\n" % (target_name, lib_path, target_name))
		clean_dir.append( "%s/lib%s.a" % (lib_path, target_name))

		makefile.write("%s/lib%s.a: $(%s)\n" % (lib_path, target_name, obj_name))
		makefile.write("\tar -cvq $@ $(%s)\n\n" % (obj_name))

def PrintBin(path, target_name, lib_name):
	
	(obj_name,inc_name, direct_inc_name, extra_cpp_flag_name) = PrintComm(path, target_name, lib_name)

	link_name = "%s_%s" % (lib_name.upper(), "LINK")
	sys_lib_name = "%s_%s" % (lib_name.upper(), "SYS_LIB")

	link_res=GetSourceTagFromDeps(path, lib_name, "LIB")
	sys_lib_res=GetSourceTagFromDeps(path, lib_name, "SYS_LIB")

	makefile.write("%s=%s\n" % (link_name, ' '.join(link_res)))
	makefile.write("%s=%s\n" % (sys_lib_name,' '.join(sys_lib_res)))

	flag_key = GetLableName(lib_name, "FLAGS")	
	makefile.write("%s+=$(LDFLAGS)\n\n" % flag_key)
	makefile.write("%s+=$(patsubst %%,-l%%, $(%s))\n" % (flag_key, link_name))
	makefile.write("%s+=$(%s)\n" % (flag_key, sys_lib_name))

	makefile.write("%s_bin:%s_dir %s\n\n" % (target_name, target_name, target_name))

	PrintReferenceDIR(target_name, direct_inc_name)

	makefile.write("%s:$(%s)\n" % (target_name, obj_name))
	makefile.write("\t$(CXX) $^ -o $@ $(%s)\n" % flag_key)
	makefile.write("\tcp $@ %s/\n\n" % sbin_path)
	clean_dir.append("%s" % target_name )
	clean_dir.append("%s/%s" % (sbin_path, target_name))

def GetSubDirList(path):
	sub_dir_list=[]

	if( os.path.exists("%s/src_list" % path ) ):
		sub_dir_file=open("%s/src_list" % path)
		lines = sub_dir_file.readlines()
		for sub_dir in lines:
			sub_dir_list.append(sub_dir.strip())
		sub_dir_file.close()
	else:
		sub_dir=os.listdir(path)
		for dir in sub_dir:
			if( os.path.isdir( "%s/%s" % ( path, dir ) ) and dir[0] != "." ):
				sub_dir_list.append( dir )
	return sub_dir_list

def PrintMakeAllSubDir(dir_list, clean_lib=False):

	if( len(dir_list) > 0 ):
		makefile.write( "SUBDIRS=%s\n\n" % ' '.join(dir_list) )

		makefile.write( ".PHONY:sub_dir\n" );
		makefile.write( "sub_dir:$(SUBDIRS)\n" );
		makefile.write("\t@for sub_dir in $^; do \\\n");
		makefile.write("\tmake -C $$sub_dir; \\\n");
		makefile.write("\tdone\n\n");
	
		makefile.write( ".PHONY:clean\n" )
		makefile.write( "clean:$(SUBDIRS)\n" )
		makefile.write("\t@for sub_dir in $^; do \\\n")
		makefile.write("\tmake -C $$sub_dir clean;\\\n")
		makefile.write("\tdone\n")
		if(clean_lib):
			makefile.write("\trm -f %s/*.a %s/*.a %s/*.a $(SRC_BASE_PATH)/lib/*.a\n" % (lib_path, ext_lib_path, sbin_path) );
		makefile.write("\trm -rf *.o *.pb.* %s " % ' '.join(clean_dir));

	else:
		makefile.write("clean:\n")
		if(clean_lib):
			makefile.write("\trm -f %s/*.a %s/*.a %s/*.a $(SRC_BASE_PATH)/lib/*.a\n" % (lib_path, ext_lib_path, sbin_path) );
		makefile.write("\trm -rf *.o *.pb.* %s " % ' '.join(clean_dir));

def Process(path, library_list, elibrary_list, binary_list):
	for lib in library_list:
		if(len(lib) == 0):
			continue
		PrintLib(path, lib, lib)
		 
	for lib in elibrary_list:
		if(len(lib) == 0):
			continue
		PrintLib(path, lib, lib, True)
	
	for lib in binary_list:
		if(len(lib) == 0):
			continue
		PrintBin(path, lib, lib)


def CreateMakeFile(path):

	global makefile
	global base_path

	target_list = []	
	library_list = []
	elibrary_list = []
	binary_list = []

	lib_count = 0

	makefile_define_path = "%s/%s" % (path, include_makefile_name)
	makefile_path = "%s/%s" % (path, makefile_name)

	makefile = open(makefile_path, "w");
	makefile.write("SRC_BASE_PATH=%s\n\n" % base_path) 
	if(os.path.exists(makefile_define_path)):
		define_makefile_file = open(makefile_define_path)
		try:
			lines = define_makefile_file.readlines()
			for line in lines:
				args = line.split('=')
				if(len(args) > 1 and args[0] == "allobject"):
					target_list = str.strip(args[1]).split(' ')
			
					for target in target_list:
						if(len(target) == 0):
							continue
						lib_count = lib_count+1
						if(target[0:3] == "lib"):
							library_list.append(target[3:-2])
						elif(target[0:4] == "elib"):
							elibrary_list.append(target[4:-2])
						else:
							binary_list.append(target)

		except:
			print("file %s not found:" % makefile_define_path)
		finally:
			define_makefile_file.close()

	makefile.write("all:");
	if( lib_count > 0 ):
		for lib in library_list:
			makefile.write("lib_%s " %(lib));

		for lib in elibrary_list:
			makefile.write("lib_%s " %(lib));

		for lib in binary_list:
			makefile.write("%s_bin " %(lib));
	else:
		makefile.write("sub_dir");
	makefile.write("\n\n");

	makefile.write("include $(SRC_BASE_PATH)/makefile.mk\n\n" )
	if( lib_count > 0 ):
		makefile.write("include %s\n" % include_makefile_name)
		Process(path[len(base_path):], library_list, elibrary_list, binary_list)

		sub_dir_list = GetSubDirList(path)
		PrintMakeAllSubDir(sub_dir_list, path==base_path)

		for target in elibrary_list:
			makefile.write("lib%s.a %s/lib%s.a " % (target, lib_path,target));
		for target in binary_list:
			makefile.write("%s " % (target));
		makefile.write("\n\n");
	else:
		sub_dir_list = GetSubDirList(path)
		PrintMakeAllSubDir(sub_dir_list, path==base_path)

	makefile.close()

	current_dir=os.path.abspath(".")


if(__name__ == '__main__'):
	base_path = sys.argv[1]
	current_path = sys.argv[2]
	if(current_path[0:len(base_path)] != base_path):
		print("path error, base %s, current %s" % (base_path, current_path[0:len(base_path)]))
		exit(0)
	CreateMakeFile(current_path)
