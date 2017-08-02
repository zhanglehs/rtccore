# !/bin/bash
if [ ! -L "build" ]; then
ln -s chromium/src/build                     build
ln -s chromium/src/testing                   testing
ln -s chromium/src/net                       net
ln -s chromium/src/base                      base
ln -s chromium/src/buildtools                buildtools
ln -s ../chromium/src/third_party/android_tools third_party/android_tools
ln -s ../chromium/src/third_party/expat         third_party/expat
ln -s ../chromium/src/third_party/libsrtp       third_party/libsrtp
ln -s ../chromium/src/third_party/usrsctp       third_party/usrsctp
ln -s ../chromium/src/third_party/libyuv        third_party/libyuv
ln -s ../chromium/src/third_party/nss           third_party/nss
ln -s ../chromium/src/third_party/jsoncpp       third_party/jsoncpp
ln -s ../chromium/src/third_party/icu           third_party/icu
ln -s ../chromium/src/third_party/jsr-305       third_party/jsr-305
ln -s ../chromium/src/third_party/libvpx        third_party/libvpx
ln -s ../chromium/src/third_party/openmax_dl    third_party/openmax_dl
ln -s ../chromium/src/third_party/boringssl     third_party/boringssl
ln -s ../chromium/src/third_party/yasm          third_party/yasm
ln -s ../chromium/src/third_party/libjpeg_turbo third_party/libjpeg_turbo
ln -s ../chromium/src/third_party/opus          third_party/opus
ln -s ../chromium/src/third_party/protobuf      third_party/protobuf
ln -s ../chromium/src/tools/android             tools/android
ln -s ../chromium/src/tools/clang               tools/clang
ln -s ../chromium/src/tools/protoc_wrapper      tools/protoc_wrapper
ln -s ../chromium/src/tools/gyp                 tools/gyp
fi
