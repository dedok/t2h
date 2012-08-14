#!/bin/bash
# Soshnikov Vasiliy <vasiliy.soshnikov@simbirsoft.com>

usage() { # print usage
	cat << EOF

$(basename $0) - prepares nDrive for building under iOS/MacOSX/BSD/Lunix/Unix-like platforms.
            In case of succeed script return 0, in case of failure 1(for details see the build.log in $SVN_TRUNK)

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

   --with-build, -with-build  set build targets [default=none], note:
	                          don't mix Mac OS X build args with iPhone Simulator build args.
										
                              macosx
                              - create folders(MacOSXi386CMake, MacOSXx86_64CMake) into
                              $SVN_TRUNK which will containt cmake files 
                              which configure to use Mac OS X cross/toolchains 

                              iossim [DEPRECATED]
                              - create folder(iPhoneSimulatorCMake) into 
                              $SVN_TRUNK which will containt cmake files 
                              which confgure to use iPhoneSimulator envt
                              
                              ios [DEPRECATED]
                              - create folder(iPhoneArmv6CMake) into 
                              $SVN_TRUNK which will containt cmake files 
                              which configure to use iPhoneOS armv6 envt
                              	
   --clear, -c                clear all previously builds 

Environment:

   APPLE_SDK_VERSION [DEPRECATED]
   version of target ios sdk version, must set for iphone os and for iphone simulator
   
Example:
   
  -- iphone simulator and iphone build --
   export APPLE_SDK_VERSION=4.3
   ./$(basename $0) --clear --build-type=Debug --with-build=ios,iossim
   
   -- mac os x build  --
   ./$(basename $0) --clear --build-type=Release --with-build=macosx

EOF
	}
	#

clear_build() { # clean-up prev build info 
	[ -z $SVN_TRUNK ] && return
	rm -Rf $SVN_TRUNK/build_dir \
        $SVN_TRUNK/lib/* \
        $SVN_TRUNK/bin/*
	}
	#

abort() { # clean-up build, print error message and exit with code 1
	clear_build
	echo "Error occurred: $@, for detail see build.log" && exit 1
	}
	#

make_build() { # cp build script 
	echo "not implemented"
	}
	#

eval_iphone_cmake() { # execute cmake command with situable args for iphone
	local tc_file=$TOOLCHAINS_DIR/$TOOLCHAIN_IPHONE_UNIVERSAL_FILE
	local cmake_basic_args="-DCMAKE_BUILD_TYPE=$1 -H$SVN_TRUNK/src -B$SVN_TRUNK/$2"
	local cmake_extra_args="-DCMAKE_TOOLCHAIN_FILE=$tc_file -DPLATFORM_NAME:STRING=$3 -DPLATFORM_ARCH:STRING=$4"
 	local cmake_sdk_args="-DSDK_ROOT:STRING=$5 -DSDK_VERSION:STRING=$6"
	local cmake_eval_cmd="cmake $cmake_extra_args $cmake_basic_args $cmake_sdk_args"
	echo "Command: $cmake_eval_cmd" && {
		(eval $cmake_eval_cmd >> $EXEC_DIR/build.log 2>&1) || abort "cmake command fail"
		}
	}
	#

eval_macosx_cmake() { # execute cmake command with situable args for macosx
	local tc_file=$TOOLCHAINS_DIR/$TOOLCHAIN_MACOSX_UNIVERSAL_FILE
	local cmake_basic_args="-DCMAKE_TOOLCHAIN_FILE=$tc_file -DCMAKE_BUILD_TYPE=$1"
	local cmake_eval_cmd="cmake -H$SVN_TRUNK/src -B$SVN_TRUNK/$2 $cmake_basic_args"
	echo  "Command: $cmake_eval_cmd" && {
		(eval $cmake_eval_cmd >> $EXEC_DIR/build.log 2>&1) || abort "cmake command fail"
		}
	}
	#

macosx() { # run macosx cmake command
	eval_macosx_cmake "$1" "build_dir"
	}
	#

iphone_sim() { # run iPhone simulator cmake command
	local cmake_work_dir=iPhoneSimCMake
	[ ! -z $APPLE_SDK_VERSION ] || abort "APPLE_SDK_VERSION as envt variable not set"
	eval_iphone_cmake "$1" "$cmake_work_dir" "iPhoneSimulator" "i386" "$IPHONEOS_APPLE_SDK_ROOT" "$APPLE_SDK_VERSION"	
	}
	#

iphone_armv6() { # run iPhoneOS armv6 cmake command
	local cmake_work_dir=iPhoneOSArmv6CMake
	[ ! -z $APPLE_SDK_VERSION ] || abort "APPLE_SDK_VERSION as envt variable not set"
	eval_iphone_cmake "$1" "$cmake_work_dir" "iPhoneOS" "armv6" "$IPHONEOS_APPLE_SDK_ROOT" "$APPLE_SDK_VERSION"	
	}
	#

iphone_armv7() { # run iPhoneOS armv6 cmake command
	local cmake_work_dir=iPhoneOSArmv7CMake
	[ ! -z $APPLE_SDK_VERSION ] || abort "APPLE_SDK_VERSION as envt variable not set"
	eval_iphone_cmake "$1" "$cmake_work_dir" "iPhoneOS" "armv7" "$IPHONEOS_APPLE_SDK_ROOT" "$APPLE_SDK_VERSION"	
	}
	#

# entry
echo $(basename $0)

EXEC_DIR=$(dirname $0)
source $EXEC_DIR/src/scripts/tools/envt-inc || abort "can not include envt"

BUILD_TYPE=Debug
BUILD_LIST=
WANNA_CLEAR=
IS_MACOS_BUILD=
IS_IPHONE_BUILD=
IS_IPHONESIM_BUILD=
IS_IPAD_BUILD=

[ -e $EXEC_DIR/build.log ] && rm -f $EXEC_DIR/build.log

for option ;do
	case $option in
		--help | -h ) 
			usage && exit 0 
			;; 
		--with-build=* | -with-build=* ) 
			[ ! -z $BUILD_LIST ] && abort "the --build-type=* option must passed through args one time"
			BUILD_LIST=$(echo `expr "x$option" : "x-*with-build=\(.*\)"` | sed "s/,/ /g")
			for x in $BUILD_LIST; do
				[ $x == "macosx" ] && IS_MACOS_BUILD=yes
				[ $x == "iossim" ] && IS_IPHONESIM_BUILD=yes
				[ $x == "ios" ] && IS_IPHONE_BUILD=yes
				[ $x == "ipad" ] && IS_IPAD_BUILD=yes
				done
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

[ ! -z $WANNA_CLEAR ] && { 
	clear_build
	echo "clean is done." 
	}

[ ! -z $IS_IPAD_BUILD ] && {
	iphone_armv7 $BUILD_TYPE
	make_build "ios"
	echo "bootstrap is done for iOS system. Type ./ios-build --help for help"
	}



[ ! -z $IS_IPHONE_BUILD ] && {
	iphone_armv6 $BUILD_TYPE
	iphone_armv7 $BUILD_TYPE
	make_build "ios"
	echo "bootstrap is done for iOS system. Type ./ios-build --help for help"
	}

[ ! -z $IS_IPHONESIM_BUILD ] && {
	iphone_sim $BUILD_TYPE
	make_build "ios"
	echo "bootstrap is done for iOS simulator system. Type ./ios-build --help for help"
	}

[ ! -z $IS_MACOS_BUILD ] && {
	macosx $BUILD_TYPE
	make_build "macosx"
	echo "bootstrap is done for Mac OS X system. Type ./mac-build --help for help"
	}
	
