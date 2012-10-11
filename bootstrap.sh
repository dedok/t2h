#!/bin/bash
# Soshnikov Vasiliy <dedok.mad@gmail.com>

# Global vars

LOG_FILE=$(basename $0).log
BUILD_SHARED_DIR=shared_build
BUILD_STATIC_DIR=static_build
ENVT_EX_SCRIPT=export_vars.sh

usage() { # print usage
	cat << EOF

$(basename $0) - prepare to build t2h under iOS/MacOSX/BSD/Lunix/Unix-like platforms.
            In case of succeed script return 0, in case of failure 1(for details see the ${LOG_FILE})

Usage: $(basename $0) [OPTIONS] ...

Defaults for the options are specified in brackets.

Options:
   --help, -h                 display usage and exit 

   --shared=[yes|no]          create an shared core library [detauld=Yes]
 
   --enable-int-workaraund    on int workaraund 

   --build-type=[D|R|R]       set build type [default=Debug]
	                         
                              Debug 
                              - generate debug build
                              
                              Release
                              - generate release build

                              RelWithDebInfo
                              - generate release with debug info

   --clear, -c                clear all previously builds 

Evnt. variables:

   BOOST_ROOT [OPTIONAL] 
     
     This variable need to the build system(CMake) to find Boost headers & libraries.   
     If Boost already into PATH then not need to set BOOST_ROOT. 
   
   LIBTORRENT_ROOT [REQUIRED] 
  
     This variable need to the build system to find headers & libraries.

Example:
  	
   export BOOST_ROOT=/home/boost_XXX
   export LIBTORRENT_ROOT=/hone/liborrent_XXX
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
	local t2h_evnt_vars="-DBOOST_ROOT=$BOOST_ROOT -DLIBTORRENT_ROOT=$LIBTORRENT_ROOT"
	local cmake_basic_args="-DCMAKE_TOOLCHAIN_FILE=$tc_file -DCMAKE_BUILD_TYPE=$1 $t2h_evnt_vars $3"
	local cmake_eval_cmd="cmake -H$REPO_ROOT/src -B$REPO_ROOT/$2 $cmake_basic_args"
	echo "Command: $cmake_eval_cmd" && {
		(eval $cmake_eval_cmd) || abort "cmake command fail"
		}
	}
	#

eval_cmake_with_shared_lib() { # execute cmake command with situable args for shared build
	local tc_file=$TOOLCHAINS_DIR/$TOOLCHAIN_MACOSX_UNIVERSAL_FILE
	local t2h_evnt_vars="-DBOOST_ROOT=$BOOST_ROOT -DLIBTORRENT_ROOT=$LIBTORRENT_ROOT"
	local cmake_basic_args="-DCMAKE_TOOLCHAIN_FILE=$tc_file -DCMAKE_BUILD_TYPE=$1 -DT2H_CORE_SHARED:BOOL=TRUE $t2h_evnt_vars $3"
	local cmake_eval_cmd="cmake -H$REPO_ROOT/src -B$REPO_ROOT/$2 $cmake_basic_args"
	echo "Command: $cmake_eval_cmd" && {
		(eval $cmake_eval_cmd) || abort "cmake command fail"
		}
	}
	#

create_build() { # run macosx cmake command
	local extra_args=
	[ x$3 = "xyes" ] \
		&& extra_args="-DT2H_INT_WORKAROUND:BOOL=TRUE"
	[ x$2 = "xyes" ] && {
		eval_cmake_with_shared_lib $1 $BUILD_SHARED_DIR $extra_args
		return
		}
		#
	eval_cmake_with_static_lib $1 $BUILD_STATIC_DIR $extra_args
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
ENABLE_INT_WORKARAUND=no

[ -e $(pwd)/$ENVT_EX_SCRIPT ] && {
	source $(pwd)/$ENVT_EX_SCRIPT
	echo "Found $pwd/$ENVT_EX_SCRIPT"
	echo "Exporting follow evnt. variables: $BOOST_ROOT $LIBTORRENT_ROOT" 
}

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
		--enable-int-workaraund | -eiw)
			ENABLE_INT_WORKARAUND=yes
			;;
		* )
			abort "unrecognized option: $option , type --help | --h to see available options"
			;;
		esac
	done

[ x$WANT_CLEAR = "xyes" ] && clear_build

create_build $BUILD_TYPE $WANT_SHARED $ENABLE_INT_WORKARAUND || \
	abort "$(basename) failed for details see ${LOG_FILE}."

configure_repo

echo "bootstrap is done."

