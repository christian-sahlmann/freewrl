#!/bin/sh

#build javascript
cd ../../freewrl/JS/js1.8/src
./build64_32_noppc.sh
cp ../../freewrl/JS/js1.8/src/Darwin_DBG.OBJ/libFreeX3Djs.dylib /usr/local/lib/libFreeX3Djs.dylib 

cd ../../../../freex3d/appleOSX/

## Make i386 library first
make distclean
cp -f vrml.conf.i386 vrml.conf.aqua
perl Makefile.PL.osx
make install
cp -f libFreeX3DFunc.dylib X3DFunc_i386.dylib

## Clean and make x86_64 library
make distclean
cp -f vrml.conf.x86_64 vrml.conf.aqua
perl Makefile.PL.osx
make install
cp -f libFreeX3DFunc.dylib X3DFunc_x86_64.dylib

## Glue the two libraries together and make sure the library has a good name
lipo -create X3DFunc_i386.dylib X3DFunc_x86_64.dylib -output libFreeX3DFunc.dylib
install_name_tool -id libFreeX3DFunc.dylib libFreeX3DFunc.dylib
cp libFreeX3DFunc.dylib /usr/local/lib/libFreeX3DFunc.dylib


## Clean up
rm -f X3DFunc_i386.dylib
rm -f X3DFunc_x86_64.dylib
