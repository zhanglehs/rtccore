#!/bin/sh

#  build.sh
#  WebRTC
#
#  Created by Rahul Behera on 6/18/14.
#  Copyright (c) 2014 Pristine, Inc. All rights reserved.

# Get location of the script itself .. thanks SO ! http://stackoverflow.com/a/246128
#SOURCE="${BASH_SOURCE[0]}"
#while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
#    DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
#    SOURCE="$(readlink "$SOURCE")"
#    [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
#done
#PROJECT_DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
PROJECT_DIR="$(pwd)"
echo PROJECT_DIR="$PROJECT_DIR"

DEPOT_TOOLS="$PROJECT_DIR/depot_tools"
BUILD="$PROJECT_DIR/out"
XCODE_SDK_PATH="/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs"
XCODE_SDK=$XCODE_SDK_PATH/iPhoneOS8.3.sdk
if [ ! -d "$XCODE_SDK" ]; then
    sudo ln -s $PROJECT_DIR/chromium/src/third_party/ios/iPhoneOS8.3.sdk $XCODE_SDK
fi
###
WEBRTC_TARGET="avengine_dll"
export PATH="$PATH":$DEPOT_TOOLS
export WEBRTC_RELEASE=true

function clear_all() {
if [ "$WEBRTC_RELEASE" ]; then
    find $PROJECT_DIR/out_ios_arm64_v8a/Release-iphoneos/ -name "*.o" | xargs rm -f
    find $PROJECT_DIR/out_ios_armeabi_v7a/Release-iphoneos/ -name "*.o" | xargs rm -f
    find $PROJECT_DIR/out_ios_ia32/Release-iphonesimulator/ -name "*.o" | xargs rm -f
    find $PROJECT_DIR/out_ios_x64/Release-iphonesimulator/ -name "*.o" | xargs rm -f

    find $PROJECT_DIR/out_ios_arm64_v8a/Release-iphoneos/ -name "*.a" | xargs rm -f
    find $PROJECT_DIR/out_ios_armeabi_v7a/Release-iphoneos/ -name "*.a" | xargs rm -f
    find $PROJECT_DIR/out_ios_ia32/Release-iphonesimulator/ -name "*.a" | xargs rm -f
    find $PROJECT_DIR/out_ios_x64/Release-iphonesimulator/ -name "*.a" | xargs rm -f
    rm -rf $BUILD
fi
if [ "$WEBRTC_DEBUG" ]; then
    find $PROJECT_DIR/out_ios_arm64_v8a/Debug-iphoneos/ -name "*.o" | xargs rm -f
    find $PROJECT_DIR/out_ios_armeabi_v7a/Debug-iphoneos/ -name "*.o" | xargs rm -f
    find $PROJECT_DIR/out_ios_ia32/Debug-iphonesimulator/ -name "*.o" | xargs rm -f
    find $PROJECT_DIR/out_ios_x64/Debug-iphonesimulator/ -name "*.o" | xargs rm -f

    find $PROJECT_DIR/out_ios_arm64_v8a/Debug-iphoneos/ -name "*.a" | xargs rm -f
    find $PROJECT_DIR/out_ios_armeabi_v7a/Debug-iphoneos/ -name "*.a" | xargs rm -f
    find $PROJECT_DIR/out_ios_ia32/Debug-iphonesimulator/ -name "*.a" | xargs rm -f
    find $PROJECT_DIR/out_ios_x64/Debug-iphonesimulator/ -name "*.a" | xargs rm -f
    rm -rf $BUILD
fi
}

function create_ninja_files(){
    rm -rf $PROJECT_DIR/out_ios_arm64_v8a/
    rm -rf $PROJECT_DIR/out_ios_armeabi_v7a/
    rm -rf $PROJECT_DIR/out_ios_ia32/
    rm -rf $PROJECT_DIR/out_ios_x64/

    cd $PROJECT_DIR

    wrios_armv7
    python webrtc/build/gyp_webrtc.py
    find out_ios_armeabi_v7a/ -name "*.ninja" | xargs sed -i "" 's/iPhoneOS10\.3/iPhoneOS8\.3/g'

    wrios_armv8
    python webrtc/build/gyp_webrtc.py
    find out_ios_arm64_v8a/ -name "*.ninja" | xargs sed -i "" 's/iPhoneOS10\.3/iPhoneOS8\.3/g'

    wrios_ia32
    python webrtc/build/gyp_webrtc.py
    find out_ios_ia32/ -name "*.ninja" | xargs sed -i "" 's/iPhoneSimulator10\.3/iPhoneSimulator8\.3/g'

    wrios_x64
    python webrtc/build/gyp_webrtc.py
    find out_ios_x64/ -name "*.ninja" | xargs sed -i "" 's/iPhoneSimulator10\.3/iPhoneSimulator8\.3/g'
}

function create_directory_if_not_found() {
    if [ ! -d "$1" ];
    then
        mkdir -v "$1"
    fi
}

function exec_libtool() {
  echo "Running libtool"
  libtool -static -v -o $@
}

function exec_strip() {
  echo "Running strip"
  strip -S -X $@
}

function exec_ninja() {
  echo "Running ninja"
  ninja -C $1 $WEBRTC_TARGET
}

create_directory_if_not_found "$PROJECT_DIR"
create_directory_if_not_found "$BUILD"

# Update/Get/Ensure the Gclient Depot Tools
function pull_depot_tools() {

    echo Get the current working directory so we can change directories back when done
    WORKING_DIR=`pwd`

    echo If no directory where depot tools should be...
    if [ ! -d "$DEPOT_TOOLS" ]
    then
        echo Make directory for gclient called Depot Tools
        mkdir -p $DEPOT_TOOLS

        echo Pull the depot tools project from chromium source into the depot tools directory
        git clone "https://chromium.googlesource.com/chromium/tools/depot_tools.git" "$DEPOT_TOOLS"

    else

        echo Change directory into the depot tools
        cd $DEPOT_TOOLS

        echo Pull the depot tools down to the latest
        git pull
    fi
    PATH="$PATH:$DEPOT_TOOLS"
    echo Go back to working directory
    cd $WORKING_DIR
}

function choose_code_signing() {
    if [ "$WEBRTC_TARGET" == "AppRTCDemo" ]; then
        echo "AppRTCDemo target requires code signing since we are building an *.ipa"
        if [[ -z $IDENTITY ]]
        then
            COUNT=$(security find-identity -v | grep -c "iPhone Developer")
            if [[ $COUNT -gt 1 ]]
            then
              security find-identity -v
              echo "Please select your code signing identity index from the above list:"
              read INDEX
              IDENTITY=$(security find-identity -v | awk -v i=$INDEX -F "\\\) |\"" '{if (i==$1) {print $3}}')
            else
              IDENTITY=$(security find-identity -v | grep "iPhone Developer" | awk -F "\) |\"" '{print $3}')
            fi
            echo Using code signing identity $IDENTITY
        fi
        sed -i -e "s/\'CODE_SIGN_IDENTITY\[sdk=iphoneos\*\]\': \'iPhone Developer\',/\'CODE_SIGN_IDENTITY[sdk=iphoneos*]\': \'$IDENTITY\',/" $PROJECT_DIR/build/common.gypi
    fi
}

# Set the base of the GYP defines, instructing gclient runhooks what to generate
function wrbase() {
    export GYP_DEFINES="build_with_libjingle=1 build_with_chromium=0 libjingle_objc=1"
    if [ "$WEBRTC_TARGET" != "AppRTCDemo" ]; then
        GYP_DEFINES="$GYP_DEFINES chromium_ios_signing=0"
    fi
    export GYP_GENERATORS="ninja,xcode-ninja"
}

# Add the iOS Device specific defines on top of the base
function wrios_armv7() {
    wrbase
    export GYP_DEFINES="$GYP_DEFINES OS=ios target_arch=arm arm_version=7"
    export GYP_GENERATOR_FLAGS="output_dir=out_ios_armeabi_v7a"
    export GYP_CROSSCOMPILE=1
}

# Add the iOS ARM 64 Device specific defines on top of the base
function wrios_armv8() {
    wrbase
    export GYP_DEFINES="$GYP_DEFINES OS=ios target_arch=arm64 target_subarch=arm64"
    export GYP_GENERATOR_FLAGS="output_dir=out_ios_arm64_v8a"
    export GYP_CROSSCOMPILE=1
}

# Add the iOS x86 Device specific defines on top of the base
function wrios_ia32() {
    wrbase
    export GYP_DEFINES="$GYP_DEFINES OS=ios target_arch=ia32"
    export GYP_GENERATOR_FLAGS="output_dir=out_ios_ia32"
    export GYP_CROSSCOMPILE=1
}

# Add the iOS x64 Device specific defines on top of the base
function wrios_x64() {
    wrbase
    export GYP_DEFINES="$GYP_DEFINES OS=ios target_arch=x64 target_subarch=arm64"
    export GYP_GENERATOR_FLAGS="output_dir=out_ios_x64"
    export GYP_CROSSCOMPILE=1
}

# Convenience function to copy the headers by creating a symbolic link to the headers directory deep within webrtc src
function copy_headers() {
#    if [ ! -h "$WEBRTC/headers" ]; then
#        create_directory_if_not_found "$BUILD"
#        ln -s "$WEBRTC/src/talk/app/webrtc/objc/public/" "$WEBRTC/headers" || true
#    fi
    echo "nothing"
}

# Build AppRTC Demo for a real device
function build_apprtc() {
    cd "$PROJECT_DIR"

    wrios_armv7
    choose_code_signing
#    gclient runhooks

    copy_headers

    if [ "$WEBRTC_DEBUG" = true ] ; then
        exec_ninja "out_ios_armeabi_v7a/Debug-iphoneos/"
        exec_libtool "$BUILD/libWebRTC-armeabi_v7a-Debug.a" $PROJECT_DIR/out_ios_armeabi_v7a/Debug-iphoneos/*.a
    fi

    if [ "$WEBRTC_PROFILE" = true ] ; then
        exec_ninja "out_ios_armeabi_v7a/Profile-iphoneos/"
        exec_libtool "$BUILD/libWebRTC-armeabi_v7a-Profile.a" $PROJECT_DIR/out_ios_armeabi_v7a/Profile-iphoneos/*.a
    fi

    if [ "$WEBRTC_RELEASE" = true ] ; then
        exec_ninja "out_ios_armeabi_v7a/Release-iphoneos/"
        exec_libtool "$BUILD/libWebRTC-armeabi_v7a-Release.a" $PROJECT_DIR/out_ios_armeabi_v7a/Release-iphoneos/*.a
        exec_strip "$BUILD/libWebRTC-armeabi_v7a-Release.a"
    fi
}


# Build AppRTC Demo for an armv7 real device
function build_apprtc_arm64() {
    cd "$PROJECT_DIR"

    wrios_armv8
    choose_code_signing
    #gclient runhooks

    copy_headers

    if [ "$WEBRTC_DEBUG" = true ] ; then
        exec_ninja "out_ios_arm64_v8a/Debug-iphoneos/"
        exec_libtool "$BUILD/libWebRTC-arm64_v8a-Debug.a" $PROJECT_DIR/out_ios_arm64_v8a/Debug-iphoneos/*.a
    fi

    if [ "$WEBRTC_PROFILE" = true ] ; then
        exec_ninja "out_ios_arm64_v8a/Profile-iphoneos/"
        exec_libtool "$BUILD/libWebRTC-arm64_v8a-Profile.a" $PROJECT_DIR/out_ios_arm64_v8a/Profile-iphoneos/*.a
    fi

    if [ "$WEBRTC_RELEASE" = true ] ; then
        exec_ninja "out_ios_arm64_v8a/Release-iphoneos/"
        exec_libtool "$BUILD/libWebRTC-arm64_v8a-Release.a" $PROJECT_DIR/out_ios_arm64_v8a/Release-iphoneos/*.a
        exec_strip "$BUILD/libWebRTC-arm64_v8a-Release.a"
    fi
}

# Build AppRTC Demo for simulator
function build_apprtc_ia32() {
    cd "$PROJECT_DIR"

    wrios_ia32
    choose_code_signing
#    gclient runhooks

    copy_headers

    if [ "$WEBRTC_DEBUG" = true ] ; then
        exec_ninja "out_ios_ia32/Debug-iphonesimulator/"
        exec_libtool "$BUILD/libWebRTC-ia32-Debug.a" $PROJECT_DIR/out_ios_ia32/Debug-iphonesimulator/*.a
    fi

    if [ "$WEBRTC_PROFILE" = true ] ; then
        exec_ninja "out_ios_ia32/Profile-iphonesimulator/"
        exec_libtool "$BUILD/libWebRTC-ia32-Profile.a" $PROJECT_DIR/out_ios_ia32/Profile-iphonesimulator/*.a
    fi

    if [ "$WEBRTC_RELEASE" = true ] ; then
        exec_ninja "out_ios_ia32/Release-iphonesimulator/"
        exec_libtool "$BUILD/libWebRTC-ia32-Release.a" $PROJECT_DIR/out_ios_ia32/Release-iphonesimulator/*.a
        exec_strip "$BUILD/libWebRTC-ia32-Release.a"
    fi
}

# Build AppRTC Demo for simulator
function build_apprtc_x64() {
    cd "$PROJECT_DIR"

    wrios_x64
    choose_code_signing
#    gclient runhooks

    copy_headers

    if [ "$WEBRTC_DEBUG" = true ] ; then
        exec_ninja "out_ios_x64/Debug-iphonesimulator/"
        exec_libtool "$BUILD/libWebRTC-x64-Debug.a" $PROJECT_DIR/out_ios_x64/Debug-iphonesimulator/*.a
    fi

    if [ "$WEBRTC_PROFILE" = true ] ; then
        exec_ninja "out_ios_x64/Profile-iphonesimulator/"
        exec_libtool "$BUILD/libWebRTC-x64-Profile.a" $PROJECT_DIR/out_ios_x64/Profile-iphonesimulator/*.a
    fi

    if [ "$WEBRTC_RELEASE" = true ] ; then
        exec_ninja "out_ios_x64/Release-iphonesimulator/"
        exec_libtool "$BUILD/libWebRTC-x64-Release.a" $PROJECT_DIR/out_ios_x64/Release-iphonesimulator/*.a
        exec_strip "$BUILD/libWebRTC-x64-Release.a"
    fi
}

# This function is used to put together the intel (simulator), armv7 and arm64 builds (device) into one static library so its easy to deal with in Xcode
# Outputs the file into the build directory with the revision number
function lipo_intel_and_arm() {
    if [ "$WEBRTC_DEBUG" = true ] ; then
        lipo_for_configuration "Debug"
    fi

    if [ "$WEBRTC_PROFILE" = true ] ; then
        lipo_for_configuration "Profile"
    fi

    if [ "$WEBRTC_RELEASE" = true ] ; then
        lipo_for_configuration "Release"
    fi
}

function lipo_for_configuration() {
    CONFIGURATION=$1

    LIPO_DIRS=""
    # Directories to use for lipo, armv7 and ia32 as default
    LIPO_DIRS="$LIPO_DIRS $BUILD/libWebRTC-armeabi_v7a-$CONFIGURATION.a"
    # Add ARM64
    LIPO_DIRS="$LIPO_DIRS $BUILD/libWebRTC-arm64_v8a-$CONFIGURATION.a"
    # Add x86
    LIPO_DIRS="$LIPO_DIRS $BUILD/libWebRTC-ia32-$CONFIGURATION.a"
    # and add x86_64
    LIPO_DIRS="$LIPO_DIRS $BUILD/libWebRTC-x64-$CONFIGURATION.a"

    # Lipo the simulator build with the ios build into a universal library
    lipo -create $LIPO_DIRS -output $BUILD/libWebRTC-arm-intel-$CONFIGURATION.a

#    exec_strip "$PROJECT_DIR/third_party/ffmpeg/ios/lib/libavcodec.a"
#    exec_strip "$PROJECT_DIR/third_party/ffmpeg/ios/lib/libavfilter.a"
#    exec_strip "$PROJECT_DIR/third_party/ffmpeg/ios/lib/libavformat.a"
#    exec_strip "$PROJECT_DIR/third_party/ffmpeg/ios/lib/libavutil.a"
#    exec_strip "$PROJECT_DIR/third_party/ffmpeg/ios/lib/libswresample.a"
#    exec_strip "$PROJECT_DIR/third_party/ffmpeg/ios/lib/libswscale.a"
#    exec_strip "$PROJECT_DIR/third_party/ffmpeg/ios/lib/libfdk-aac.a"
#    exec_strip "$PROJECT_DIR/third_party/ffmpeg/ios/lib/libx264.a"
    exec_libtool $BUILD/libWebRTC-$CONFIGURATION.a $BUILD/libWebRTC-arm-intel-$CONFIGURATION.a $PROJECT_DIR/third_party/ffmpeg/ios/lib/*.a
}

# Build webrtc for an ios device and simulator, then create a universal library
function build_webrtc() {
#    pull_depot_tools
    rm $BUILD/libWebRTC-*.a
    build_apprtc
    build_apprtc_arm64
    build_apprtc_ia32
    build_apprtc_x64
    lipo_intel_and_arm
}
