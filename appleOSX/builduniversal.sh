#!/bin/sh

## Make i386 library first
make distclean
cp -f vrml.conf.i386 vrml.conf.aqua
perl Makefile.PL.osx
make install
cp -f libFreeWRLFunc.dylib VRMLFunc_i386.dylib

## Clean and make ppc library
make distclean
cp -f vrml.conf.x86_64 vrml.conf.aqua
perl Makefile.PL.osx

## Have to change gcc version here, as MakeMaker doesn't let you pass it as an option?
make install
cp -f libFreeWRLFunc.dylib VRMLFunc_x86_64.dylib

## Glue the two libraries together and make sure the library has a good name
lipo -create VRMLFunc_i386.dylib VRMLFunc_x86_64.dylib -output libFreeWRLFunc.dylib
install_name_tool -id libFreeWRLFunc.dylib libFreeWRLFunc.dylib
cp libFreeWRLFunc.dylib /usr/local/lib/libFreeWRLFunc.dylib

#build javascript
../../freewrl/JS/js1.8/src/build64_32.sh
cp ../../freewrl/JS/js1.8/src/Darwin_DBG.OBJ/libjs.dylib /usr/local/lib/libjs.dylib 

## Clean up
rm -f VRMLFunc_i386.dylib
rm -f VRMLFunc_x86_64.dylib
rm -f libFreeWRLFunc_i386.so
rm -f libFreeWRLFunc_x86_64.so

