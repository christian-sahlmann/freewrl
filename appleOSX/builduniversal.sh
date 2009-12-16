#!/bin/sh

echo "please do not do this - do the intel one only for now (sh buildi386.sh)"
#
## Make i386 library first
#make distclean
#cp -f JS/js1.5/src/config/Darwin.mk.i386 JS/js1.5/src/config/Darwin.mk
#cp -f JS/js1.5/src/editline/Makefile.ref.i386 JS/js1.5/src/editline/Makefile.ref
#cp -f JS/js1.5/src/Makefile.ref.uni JS/js1.5/src/Makefile.ref
#cp -f JS/js1.5/src/Makefile.in.uni JS/js1.5/src/Makefile.in
#cp -f JS/js1.5/src/rules.mk.uni JS/js1.5/src/rules.mk
#cp -f vrml.conf.i386 vrml.conf.aqua
#perl Makefile.PL.osx
#make install
#cp -f libFreeWRLFunc.dylib VRMLFunc_i386.dylib
#cp -f JS/js1.5/src/Darwin_OPT.OBJ/libFreeWRLjs.so libFreeWRLjs_i386.so
#
## Clean and make ppc library
#make distclean
#cp -f vrml.conf.ppc vrml.conf.aqua
#cp -f JS/js1.5/src/config/Darwin.mk.ppc JS/js1.5/src/config/Darwin.mk
#cp -f JS/js1.5/src/editline/Makefile.ref.ppc JS/js1.5/src/editline/Makefile.ref
#perl Makefile.PL.osx
#
## Have to change gcc version here, as MakeMaker doesn't let you pass it as an option?
#make install
#cp -f libFreeWRLFunc.dylib VRMLFunc_ppc.dylib
#cp -f JS/js1.5/src/Darwin_OPT.OBJ/libFreeWRLjs.so libFreeWRLjs_ppc.so
#
## Glue the two libraries together and make sure the library has a good name
#lipo -create VRMLFunc_i386.dylib VRMLFunc_ppc.dylib -output libFreeWRLFunc.dylib
#install_name_tool -id libFreeWRLFunc.dylib libFreeWRLFunc.dylib
#cp libFreeWRLFunc.dylib /usr/local/lib/libFreeWRLFunc.dylib
#
#lipo -create libFreeWRLjs_i386.so libFreeWRLjs_ppc.so -output libFreeWRLjs.dylib
#cp libFreeWRLjs.dylib /usr/local/lib/libFreeWRLjs.dylib
#
## Clean up
#rm -f VRMLFunc_i386.dylib
#rm -f VRMLFunc_ppc.dylib
#rm -f libFreeWRLjs.dylib
#rm -f libFreeWRLFunc.dylib
#rm -f libFreeWRLFunc_i386.so
#rm -f libFreeWRLFunc_ppc.so
