#!/bin/sh

#use this when there are a few changes to be made; it is a simple quick build; not full build.

make install

# Glue the two libraries together and make sure the library has a good name
install_name_tool -id libFreeWRLFunc.dylib libFreeWRLFunc.dylib
cp libFreeWRLFunc.dylib /usr/local/lib/libFreeWRLFunc.dylib


