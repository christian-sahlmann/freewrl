#!/bin/sh

make distclean
cp -f vrml.conf.iphone vrml.conf.aqua
perl Makefile.PLStatic.osx
make install

# Glue the two libraries together and make sure the library has a good name
install_name_tool -id libFreeWRLFunc.dylib libFreeWRLFunc.dylib
cp libFreeWRLFunc.dylib /usr/local/lib/libFreeWRLFunc.dylib

# Clean up
#rm -f libFreeWRLFunc.dylib
