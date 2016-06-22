import os
import sys

include_makefile_name="Makefile.define"
user_home = os.path.expanduser('~')

def GetLableName( name, suffix ):
	return name.upper()+"_"+suffix

def GetLibName( name ):
	return "lib"+name+".a"

def GetELibName( name ):
	return "elib"+name+".a"

