#!/bin/bash

DEVELOPER="/Applications/Xcode.app/Contents/Developer"
PLATFORM="iPhoneOS"
SDK="iPhoneOS6.1"
ARCHS="armv7 armv7s"

LLVM="${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/usr/bin/llvm-gcc-4.2"
ISYSROOT="${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/SDKs/${SDK}.sdk"

mkdir -p bin

OUTFILES=

echo "Building..."
for ARCH in ${ARCHS}
do
    ${LLVM} -isysroot ${ISYSROOT} -arch ${ARCH} -I./ -IAirFloat/ -std=c99 -lobjc -Wall -framework CFNetwork -framework CoreFoundation -framework AudioToolbox ./AirFloat/*.c AirFloatDaemon/main.c libcrypto.a -o bin/AirFloatDaemon-${ARCH}
    OUTFILES="${OUTFILES} bin/AirFloatDaemon-${ARCH}"
done

echo "Creating fat binary..."
lipo ${OUTFILES} -create -output bin/AirFloatDaemon

echo "Cleaning up..."
rm ${OUTFILES}
