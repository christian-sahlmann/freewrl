#!/bin/sh

#use this when there are a few changes to be made; it is a simple quick build; not full build.

make install

# Glue the two libraries together and make sure the library has a good name
install_name_tool -id libFreeX3DFunc.dylib libFreeX3DFunc.dylib
cp libFreeX3DFunc.dylib /usr/local/lib/libFreeX3DFunc.dylib


