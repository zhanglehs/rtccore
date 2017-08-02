# !/bin/bash
# Copyright Pristine Inc 
# Author: Rahul Behera <rahul@pristine.io>
# Author: Aaron Alaniz <aaron@pristine.io>
# Author: Arik Yaacob   <arik@pristine.io>
#
# Builds the android peer connection library

# Get location of the script itself .. thanks SO ! http://stackoverflow.com/a/246128
#SOURCE="${BASH_SOURCE[0]}"
#while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
#    DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
#    SOURCE="$(readlink "$SOURCE")"
#    [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
#done
#PROJECT_ROOT="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
PROJECT_ROOT="$(pwd)"

# Utility method for creating a directory
create_directory_if_not_found() {
	# if we cannot find the directory
	if [ ! -d "$1" ];
		then
		echo "$1 directory not found, creating..."
	    mkdir -p "$1"
	    echo "directory created at $1"
	fi
}


#DEFAULT_WEBRTC_URL="https://chromium.googlesource.com/external/webrtc"
#DEPOT_TOOLS="$PROJECT_ROOT/depot_tools"
#WEBRTC_ROOT="$PROJECT_ROOT/webrtc"
#create_directory_if_not_found $WEBRTC_ROOT
BUILD="$PROJECT_ROOT/out"
create_directory_if_not_found $BUILD
WEBRTC_TARGET="avengine_dll"
#WEBRTC_TARGET="WebRTCDemo"

ANDROID_TOOLCHAINS="$PROJECT_ROOT/third_party/android_tools/ndk/toolchains"

#
NDK=$PROJECT_ROOT/third_party/android_tools/ndk
NDK_ROOT=$NDK
SDK=$PROJECT_ROOT/third_party/android_tools/sdk
ANTTOOLS=$SDK/tools
AAPT=$SDK/build-tools/14.0.0
CLASS_PATH=$PROJECT_ROOT/third_party/android_tools/sdk/platforms/android-14/android.jar
SYSROOT=$NDK/platforms/android-16/arch-arm/
SYSROOT2=$NDK/platforms/android-21/arch-arm64/
TOOLCHAIN=$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64
TOOLCHAIN2=$NDK/toolchains/aarch64-linux-androideabi-4.9/prebuilt/linux-x86_64

export PATH="$PATH":$PROJECT_ROOT/depot_tools
export WEBRTC_ARCH=armv7
#export WEBRTC_ARCH=armv8
export CLASSPATH="$CLASS_PATH":.

function clear_all(){
    rm -rf $PROJECT_ROOT/out_android_armeabi-v7a
}
#===============
exec_ninja() {
  echo "Running ninja"
  echo "$WEBRTC_TARGET"
  ninja -C $1 $WEBRTC_TARGET
}

# Prepare our build
function wrbase() {
    export GYP_DEFINES="OS=android host_os=linux libjingle_java=1 build_with_libjingle=1 build_with_chromium=0 enable_tracing=1 enable_android_opensl=0"
    export GYP_GENERATORS="ninja"
}

# Arm V7 with Neon
function wrarmv7() {
    wrbase
    export GYP_DEFINES="$GYP_DEFINES OS=android"
    export GYP_GENERATOR_FLAGS="$GYP_GENERATOR_FLAGS output_dir=out_android_armeabi-v7a"
    export GYP_CROSSCOMPILE=1
    echo "ARMv7 with Neon Build"
}

# Arm V7 with Neon
function wrarmv7_x() {
    wrbase
    export GYP_DEFINES="$GYP_DEFINES OS=android"
    export GYP_GENERATOR_FLAGS="output_dir=out_android_armeabi-v7a enable_protobuf=0"
    export GYP_CROSSCOMPILE=1
    echo "ARMv7 with Neon Build"
}

# Arm V8 with Neon
function wrarmv8() {
    wrbase
    export GYP_DEFINES="$GYP_DEFINES OS=android"
    export GYP_GENERATOR_FLAGS="$GYP_GENERATOR_FLAGS output_dir=out_android_arm64-v8a"
    export GYP_CROSSCOMPILE=1
    echo "ARMv8 with Neon Build"
}

# Setup our defines for the build
prepare_gyp_defines() {
    # Configure environment for Android
    echo Setting up build environment for Android
    source $PROJECT_ROOT/build/android/envsetup.sh

    # Check to see if the user wants to set their own gyp defines
    # echo Export the base settings of GYP_DEFINES so we can define how we want to build
#    if [ -z $USER_GYP_DEFINES ]
#    then
        # echo "User has not specified any gyp defines so we proceed with default"
        if [ "$WEBRTC_ARCH" = "armv7" ] ;
        then
            wrarmv7
        elif [ "$WEBRTC_ARCH" = "armv8" ] ;
        then
            wrarmv8
        fi
#    else
#        echo "User has specified their own gyp defines"
#        export GYP_DEFINES="$USER_GYP_DEFINES"
#    fi

    echo "GYP_DEFINES=$GYP_DEFINES"
}

# Builds the apprtc demo
build_webrtc() {
    WORKING_DIR=`pwd`
    cd "$PROJECT_ROOT"

    echo Run gclient hooks edited by gxh
# gclient runhooks

    if [ "$WEBRTC_ARCH" = "armv7" ] ;
    then
        ARCH="armeabi-v7a"
        STRIP=$ANDROID_TOOLCHAINS/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-strip
    elif [ "$WEBRTC_ARCH" = "armv8" ] ;
    then
        ARCH="arm64-v8a"
        STRIP=$ANDROID_TOOLCHAINS/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-strip
    fi

    if [ "$WEBRTC_DEBUG" = "true" ] ;
    then
        BUILD_TYPE="Debug"
    else
        BUILD_TYPE="Release"
    fi

    ARCH_OUT="out_android_${ARCH}"
    echo "Build ${WEBRTC_TARGET} in $BUILD_TYPE (arch: ${WEBRTC_ARCH:-arm})"
    echo "$ARCH_OUT/$BUILD_TYPE"
    rm -rf $BUILD/*
    exec_ninja "$ARCH_OUT/$BUILD_TYPE"
    # Verify the build actually worked
    if [ $? -eq 0 ]; then
        SOURCE_DIR="$PROJECT_ROOT/$ARCH_OUT/$BUILD_TYPE"
        cp $SOURCE_DIR/lib.java/*_java.jar "$BUILD"
        cp "$SOURCE_DIR/libavengine_dll.so" "$BUILD"

#         $STRIP -o $ARCH_JNI/libjingle_peerconnection_so.so $WEBRTC_ROOT/src/$ARCH_OUT/$BUILD_TYPE/lib/libjingle_peerconnection_so.so -s    

        echo "$BUILD_TYPE build for apprtc complete"
    else
        echo "$BUILD_TYPE build for apprtc failed"
    fi
	cd $WORKING_DIR
}

# Updates webrtc and builds apprtc
create_ninja_files() {
    echo Setting up build environment for Android
    source $PROJECT_ROOT/build/android/envsetup.sh

    # Check to see if the user wants to set their own gyp defines
    echo "User has not specified any gyp defines so we proceed with default"
    if [ "$WEBRTC_ARCH" = "armv7" ] ;
    then
        wrarmv7_x
    elif [ "$WEBRTC_ARCH" = "armv8" ] ;
    then
        wrarmv8_x
    fi
    # python $PROJECT_ROOT/webrtc/build/gyp_webrtc.py
    cd $PROJECT_ROOT
    python webrtc/build/gyp_webrtc.py
}

prepare_gyp_defines

