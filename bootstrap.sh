#!/bin/bash
# Soshnikov Vasiliy <dedok.mad@gmail.com>

# Global vars

LOG_FILE=$(basename $0).log
BUILD_SHARED_DIR=shared_build
BUILD_STATIC_DIR=static_build

usage() { # print usage
	cat << EOF

$(basename $0) - prepares nDrive for building under iOS/MacOSX/BSD/Lunix/Unix-like platforms.
            In case of succeed script return 0, in case of failure 1(for details see the ${LOG_FILE})

Usage: $(basename $0) [OPTIONS] ...

Defaults for the options are specified in brackets.

Options:
   --help, -h                 display usage and exit 

   --shared=[yes|no]          create an shared core library [detauld=Yes]

   --build-type=[D|R|R]       set build type [default=Debug]
	                         
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
	rm -Rf $BUILD_STATIC_DIR $BUILD_SHARED_DIR $REPO_ROOT/lib $REPO_ROOT/bin
	}
	#

abort() { # clean-up build, print error message and exit with code 1
	clear_build
	echo "Error occurred: $@, for detail see ${LOG_FILE}" && exit 1
	}
	#

eval_cmake_with_static_lib() { # execute cmake command with situable args for static build
	local tc_file=$TOOLCHAINS_DIR/$TOOLCHAIN_MACOSX_UNIVERSAL_FILE
	local cmake_basic_args="-DCMAKE_TOOLCHAIN_FILE=$tc_file -DCMAKE_BUILD_TYPE=$1"
	local cmake_eval_cmd="cmake -H$REPO_ROOT/src -B$REPO_ROOT/$2 $cmake_basic_args"
	echo "Command: $cmake_eval_cmd" && {
		(eval $cmake_eval_cmd) || abort "cmake command fail"
		}
	}
	#

eval_cmake_with_shared_lib() { # execute cmake command with situable args for shared build
	local tc_file=$TOOLCHAINS_DIR/$TOOLCHAIN_MACOSX_UNIVERSAL_FILE
	local cmake_basic_args="-DCMAKE_TOOLCHAIN_FILE=$tc_file -DCMAKE_BUILD_TYPE=$1 -DT2H_CORE_SHARED:BOOL=TRUE"
	local cmake_eval_cmd="cmake -H$REPO_ROOT/src -B$REPO_ROOT/$2 $cmake_basic_args"
	echo "Command: $cmake_eval_cmd" && {
		(eval $cmake_eval_cmd) || abort "cmake command fail"
		}
	}
	#

create_build() { # run macosx cmake command
	[ x$2 = "xyes" ] && {
		eval_cmake_with_shared_lib $1 $BUILD_SHARED_DIR
		return
		}
	eval_cmake_with_static_lib $1 $BUILD_STATIC_DIR
	}
	#

configure_repo() { # pre-build build configurations
	[ ! -e $REPO_ROOT/lib ] && mkdir $REPO_ROOT/lib
	[ ! -e $REPO_ROOT/bin ] && mkdir $REPO_ROOT/bin
	}
	#

# entry
echo $(basename $0)

EXEC_DIR=$(dirname $0)
source $EXEC_DIR/src/scripts/tools/envt-inc || abort "can not include envt"

BUILD_TYPE=Debug
BUILD_LIST=
WANT_CLEAR=
WANT_SHARED=no
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
			WANT_CLEAR=yes
			;;
		--shared=*)
			WANT_SHARED=`expr "x$option" : "x--shared=\(.*\)"`
			;;
		* )
			abort "unrecognized option: $option , type --help | --h to see available options"
			;;
		esac
	done

[ x$WANT_CLEAR = "xyes" ] && clear_build

create_build $BUILD_TYPE $WANT_SHARED || \
	abort "$(basename) failed for details see ${LOG_FILE}."

configure_repo

echo "bootstrap is done."

