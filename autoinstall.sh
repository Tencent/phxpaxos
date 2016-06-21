#!/bin/bash

base_dir=`pwd`
make_file_tools=$base_dir/tools/create_makefile.py
check_env_tools=$base_dir/tools/check_install.py
src_dir=$base_dir/src_list

function check_env(){
	python $check_env_tools $base_dir
	if [ $? -gt 0 ];
	then
		exit 1
	fi
}

function create_makefile(){
	python $make_file_tools $base_dir $1 
}

function scandir(){
	if [ $1 ];then
		echo "[creating makefile] $1"
		create_makefile $1
		for file in `ls $1`
		do
			if ([ -d $1"/"$file ] && [ "$file" != "glog" ])
				then
					scandir $1"/"$file
			fi
		done
	fi
}

function process(){
	create_makefile $base_dir
	res=`cat $src_dir`
	echo $res
	for file in $res
	do
		if ([ -d $base_dir"/"$file ] && [ "$file" != "glog" ])
			then
				scandir $base_dir"/"$file
		fi
	done
}

function check(){
	make verify-install --file=makefile.mk
	if [ $? -eq 0 ]; then
		return 0
	else
		echo "install fail"
		return 1
	fi
}

function showusage(){
	echo "Configuration:"
	echo "  -h, --help              display this help and exit"
	echo "Installation directories:"
	echo "  --prefix=PREFIX         install architecture-independent files in PREFIX"
	echo "                          [.]"
	exit
}

prefix=

ARGS=`getopt -o h -l prefix:,help -- "$@"`  
eval set -- "${ARGS}" 

while true  
do  
	case "$1" in 
		--prefix)  
			if [ $2 ]; then
				sed -i -r "s#PREFIX=.*#PREFIX=$2#" makefile.mk
			fi
			shift  
			;;  
		-h|--help)
			showusage
			break
			;;
		--)  
			shift  
			break 
			;;  
	esac  
shift  
done 

check_env
echo "check evn done"
#check 
if [ $? -eq 0 ]; then 
process 
fi
