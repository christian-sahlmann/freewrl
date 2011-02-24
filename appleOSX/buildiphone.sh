#!/bin/sh

cp -f vrml.conf.iphone vrml.conf.aqua

make distclean
perl Makefile.PLStatic.osx
make install

