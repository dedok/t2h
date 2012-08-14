#!/bin/bash
# Soshnikov Vasiliy <dedok.mad@gmail.com>

# Global vars

BUILD_DIR=build_dir
LOG_FILE=$(basename $0).log

usage() { # print usage
	cat << EOF

$(basename $0) - prepares nDrive for building under iOS/MacOSX/BSD/Lunix/Unix-like platforms.
            In case of succeed script return 0, in case of failure 1(for details see the ${LOG_FILE})

Usage: $(basename $0) [OPTIONS] ...

Defaults for the options are specified in brackets.

Options:
   --help, -h                 display usage and exit 

   --build-type               set build type [default=Debug]
	                         
                              Debug 
                              - generate debug build
                              
                              Release
                              - generate release build

                              RelWithDebInfo
                              - generate release with debug info

   --clear, -c                clear all previously builds 

Example:
   
   ./$(basename $0) --clear --build-type=Release

EOF
	}
	#

clear_build() { # clean-up prev build info 
	rm -Rf $BUILD_DIR $SVN_TRUNK/lib/* $SVN_TRUNK/bin/* $SVN_TRUNK/bs
	}
	#

abort() { # clean-up build, print error message and exit with code 1
	clear_build
	echo "Error occurred: $@, for detail see ${LOG_FILE}" && exit 1
	}
	#

eval_cmake() { # execute cmake command with situable args for macosx
	local tc_file=$TOOLCHAINS_DIR/$TOOLCHAIN_MACOSX_UNIVERSAL_FILE
	local cmake_basic_args="-DCMAKE_TOOLCHAIN_FILE=$tc_file -DCMAKE_BUILD_TYPE=$1"
	local cmake_eval_cmd="cmake -H$SVN_TRUNK/src -B$SVN_TRUNK/$2 $cmake_basic_args"
	echo  "Command: $cmake_eval_cmd" && {
		(eval $cmake_eval_cmd) || abort "cmake command fail"
		}
	}
	#

create_build() { # run macosx cmake command
	eval_cmake $1 $BUILD_DIR
	}
	#

# entry
echo $(basename $0)

EXEC_DIR=$(dirname $0)
source $EXEC_DIR/src/scripts/tools/envt-inc || abort "can not include envt"

BUILD_TYPE=Debug
BUILD_LIST=
WANNA_CLEAR=
IS_IPAD_BUILD=

for option ;do
	case $option in
		--help | -h ) 
			usage && exit 0 
			;; 
		--with-build=* | -with-build=* ) 
			BUILD_LIST=$(echo `expr "x$option" : "x-*with-build=\(.*\)"` | sed "s/,/ /g")	
			abort "the --with-build=* command line option deprecated"
			;;
		--build-type=* )
			BUILD_TYPE=`expr "x$option" : "x--build-type=\(.*\)"`
			;;
		--clear | -c )
			WANNA_CLEAR=yes
			;;
		* )
			abort "unrecognized option: $option , type --help | --h to see available options"
			;;
		esac
	done

[ ! -z $WANNA_CLEAR ] && clear_build

create_build $BUILD_TYPE || abort "$(basename) failed for details see ${LOG_FILE}."
	
echo "bootstrap is done."

