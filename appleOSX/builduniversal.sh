#!/bin/sh

#build javascript
cd ../../freewrl/JS/js1.8/src
./build64_32.sh
cp ../../freewrl/JS/js1.8/src/Darwin_DBG.OBJ/libjs.dylib /usr/local/lib/libjs.dylib 

cd ../../../../freex3d/appleOSX/

## Make i386 library first
make distclean
rm -f ../src/lib/non_web3d_formats/*.o
rm -f ../src/message/*.o
cp -f vrml.conf.i386 vrml.conf.aqua
perl Makefile.PL.osx
make install
cp -f libFreeWRLFunc.dylib VRMLFunc_i386.dylib

## Clean and make x86_64 library
make distclean
rm -f ../src/lib/non_web3d_formats/*.o
rm -f ../src/message/*.o
cp -f vrml.conf.x86_64 vrml.conf.aqua
perl Makefile.PL.osx
make install
cp -f libFreeWRLFunc.dylib VRMLFunc_x86_64.dylib

## Clean and make ppc library
make distclean
rm -f ../src/lib/non_web3d_formats/*.o
rm -f ../src/message/*.o
cp -f vrml.conf.ppc vrml.conf.aqua
perl Makefile.PL.osx
make install
cp -f libFreeWRLFunc.dylib VRMLFunc_ppc.dylib

## Glue the two libraries together and make sure the library has a good name
lipo -create VRMLFunc_i386.dylib VRMLFunc_x86_64.dylib VRMLFunc_ppc.dylib -output libFreeWRLFunc.dylib
install_name_tool -id libFreeWRLFunc.dylib libFreeWRLFunc.dylib
cp libFreeWRLFunc.dylib /usr/local/lib/libFreeWRLFunc.dylib


## Clean up
rm -f VRMLFunc_i386.dylib
rm -f VRMLFunc_x86_64.dylib
rm -f VRMLFunc_ppc.dylib
