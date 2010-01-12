#!/bin/sh

# Make i386 library first
make distclean
cp -f ../../freewrl/JS/js1.5/src/config/Darwin.mk.i386 ../../freewrl/JS/js1.5/src/config/Darwin.mk
cp -f ../../freewrl/JS/js1.5/src/editline/Makefile.ref.i386 ../../freewrl/JS/js1.5/src/editline/Makefile.ref
cp -f ../../freewrl/JS/js1.5/src/Makefile.ref.uni ../../freewrl/JS/js1.5/src/Makefile.ref
cp -f ../../freewrl/JS/js1.5/src/Makefile.in.uni ../../freewrl/JS/js1.5/src/Makefile.in
cp -f ../../freewrl/JS/js1.5/src/rules.mk.uni ../../freewrl/JS/js1.5/src/rules.mk
cp -f vrml.conf.iphone vrml.conf.aqua
perl Makefile.PL.osx
make install
cp -f libFreeWRLFunc.dylib VRMLFunc_i386.dylib
cp -f ../../freewrl/JS/js1.5/src/Darwin_OPT.OBJ/libFreeWRLjs.so libFreeWRLjs_i386.so

# Glue the two libraries together and make sure the library has a good name
lipo -create VRMLFunc_i386.dylib -output libFreeWRLFunc.dylib
install_name_tool -id libFreeWRLFunc.dylib libFreeWRLFunc.dylib
cp libFreeWRLFunc.dylib /usr/local/lib/libFreeWRLFunc.dylib

lipo -create libFreeWRLjs_i386.so -output libFreeWRLjs.dylib
cp libFreeWRLjs.dylib /usr/local/lib/libFreeWRLjs.dylib

# Clean up
rm -f VRMLFunc_i386.dylib
rm -f libFreeWRLjs.dylib
rm -f libFreeWRLFunc.dylib
rm -f libFreeWRLFunc_i386.so
