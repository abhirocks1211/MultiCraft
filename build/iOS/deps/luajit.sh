#!/bin/bash -e

. sdk.sh
LUAJIT_VERSION=2.1

if [ ! -d luajit-src ]; then
	wget https://github.com/LuaJIT/LuaJIT/archive/v$LUAJIT_VERSION.zip
	unzip v$LUAJIT_VERSION.zip
	mv LuaJIT-$LUAJIT_VERSION luajit-src
	rm v$LUAJIT_VERSION.zip
fi

cd luajit-src

# 32-bit
make -j$(sysctl -n hw.ncpu) \
  DEFAULT_CC=clang HOST_CC="clang -m32 -arch i386" CROSS="$(dirname $IOS_CC)/" \
  TARGET_FLAGS="${IOS_FLAGS_LUA/-arch arm64/}" TARGET_SYS=iOS \
  -j$(sysctl -n hw.ncpu)
mv src/libluajit.a tmp32.a
make clean
# 64-bit
make -j$(sysctl -n hw.ncpu) \
  DEFAULT_CC=clang HOST_CC=clang CROSS="$(dirname $IOS_CC)/" \
  TARGET_FLAGS="${IOS_FLAGS_LUA/-arch armv7/}" TARGET_SYS=iOS \
  -j$(sysctl -n hw.ncpu)
mv src/libluajit.a tmp64.a
make clean
# repack into one .a
lipo tmp32.a tmp64.a -create -output libluajit.a
rm tmp32.a tmp64.a

mkdir -p ../luajit/{lib,include}
cp -v src/*.h ../luajit/include
cp -v libluajit.a ../luajit/lib

echo "LuaJIT build successful"
