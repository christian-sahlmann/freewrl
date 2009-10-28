# brute force compile the eailib.dylib on OSX
#

# first, the PPC version

gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch ppc -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.3.9.sdk \
	-IOSX_Specific \
	-I../freex3d/src/lib \
	-I../freex3d/src/lib/input \
        -I../freex3d/src/libeai \
	../freex3d/src/libeai/EAI_C_Control.c -o ../freex3d/src/libeai/EAI_C_Control.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch ppc -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.3.9.sdk \
	-IOSX_Specific \
	-I../freex3d/src/lib \
	-I../freex3d/src/lib/input \
        -I../freex3d/src/libeai \
	../freex3d/src/libeai/EAI_C_Field.c -o ../freex3d/src/libeai/EAI_C_Field.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch ppc -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.3.9.sdk \
	-IOSX_Specific \
	-I../freex3d/src/lib \
	-I../freex3d/src/lib/input \
        -I../freex3d/src/libeai \
	../freex3d/src/libeai/EAI_C_Advise.c -o ../freex3d/src/libeai/EAI_C_Advise.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch ppc -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.3.9.sdk \
	-IOSX_Specific \
	-I../freex3d/src/lib \
	-I../freex3d/src/lib/input \
        -I../freex3d/src/libeai \
	../freex3d/src/libeai/EAI_C_Internals.c -o ../freex3d/src/libeai/EAI_C_Internals.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch ppc -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.3.9.sdk \
	-IOSX_Specific \
	-I../freex3d/src/lib \
	-I../freex3d/src/lib/input \
        -I../freex3d/src/libeai \
	../freex3d/src/libeai/EAI_C_Node.c -o ../freex3d/src/libeai/EAI_C_Node.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch ppc -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.3.9.sdk \
	-IOSX_Specific \
	-I../freex3d/src/lib \
	-I../freex3d/src/lib/input \
        -I../freex3d/src/libeai \
	../freex3d/src/libeai/EAI_C_ReWire.c -o ../freex3d/src/libeai/EAI_C_ReWire.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch ppc -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.3.9.sdk \
	-IOSX_Specific \
	-I../freex3d/src/lib \
	-I../freex3d/src/lib/input \
        -I../freex3d/src/libeai \
	../freex3d/src/libeai/GeneratedCode.c -o ../freex3d/src/libeai/GeneratedCode.o 

gcc -dynamiclib  \
	-arch ppc \
	-L/usr/local/lib -lFreeWRLFunc \
	../freex3d/src/libeai/EAI_C_Control.o \
	../freex3d/src/libeai/EAI_C_Field.o \
	../freex3d/src/libeai/EAI_C_Advise.o \
	../freex3d/src/libeai/EAI_C_Internals.o \
	../freex3d/src/libeai/EAI_C_Node.o \
	../freex3d/src/libeai/EAI_C_ReWire.o \
	../freex3d/src/libeai/GeneratedCode.o \
	-o libEAI.ppc

rm 	../freex3d/src/libeai/EAI_C_Control.o 
rm 	../freex3d/src/libeai/EAI_C_Field.o 
rm 	../freex3d/src/libeai/EAI_C_Advise.o 
rm 	../freex3d/src/libeai/EAI_C_Internals.o 
rm 	../freex3d/src/libeai/EAI_C_Node.o 
rm 	../freex3d/src/libeai/EAI_C_ReWire.o 
rm 	../freex3d/src/libeai/GeneratedCode.o 

gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch i386 -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.4u.sdk \
	-IOSX_Specific \
	-I../freex3d/src/lib \
	-I../freex3d/src/lib/input \
        -I../freex3d/src/libeai \
	../freex3d/src/libeai/EAI_C_Control.c -o ../freex3d/src/libeai/EAI_C_Control.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch i386 -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.4u.sdk \
	-IOSX_Specific \
	-I../freex3d/src/lib \
	-I../freex3d/src/lib/input \
        -I../freex3d/src/libeai \
	../freex3d/src/libeai/EAI_C_Field.c -o ../freex3d/src/libeai/EAI_C_Field.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch i386 -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.4u.sdk \
	-IOSX_Specific \
	-I../freex3d/src/lib \
	-I../freex3d/src/lib/input \
        -I../freex3d/src/libeai \
	../freex3d/src/libeai/EAI_C_Advise.c -o ../freex3d/src/libeai/EAI_C_Advise.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch i386 -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.4u.sdk \
	-IOSX_Specific \
	-I../freex3d/src/lib/input \
	-I../freex3d/src/lib \
        -I../freex3d/src/libeai \
	../freex3d/src/libeai/EAI_C_Internals.c -o ../freex3d/src/libeai/EAI_C_Internals.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch i386 -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.4u.sdk \
	-IOSX_Specific \
	-I../freex3d/src/lib \
	-I../freex3d/src/lib/input \
        -I../freex3d/src/libeai \
	../freex3d/src/libeai/EAI_C_Node.c -o ../freex3d/src/libeai/EAI_C_Node.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch i386 -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.4u.sdk \
	-IOSX_Specific \
	-I../freex3d/src/lib \
	-I../freex3d/src/lib/input \
        -I../freex3d/src/libeai \
	../freex3d/src/libeai/EAI_C_ReWire.c -o ../freex3d/src/libeai/EAI_C_ReWire.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch i386 -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.4u.sdk \
	-IOSX_Specific \
	-I../freex3d/src/lib \
	-I../freex3d/src/lib/input \
        -I../freex3d/src/libeai \
	../freex3d/src/libeai/GeneratedCode.c -o ../freex3d/src/libeai/GeneratedCode.o 

gcc -dynamiclib  \
	-L/usr/local/lib -lFreeWRLFunc \
	../freex3d/src/libeai/EAI_C_Control.o \
	../freex3d/src/libeai/EAI_C_Field.o \
	../freex3d/src/libeai/EAI_C_Advise.o \
	../freex3d/src/libeai/EAI_C_Internals.o \
	../freex3d/src/libeai/EAI_C_Node.o \
	../freex3d/src/libeai/EAI_C_ReWire.o \
	../freex3d/src/libeai/GeneratedCode.o \
	-o libEAI.i386

rm 	../freex3d/src/libeai/EAI_C_Control.o 
rm 	../freex3d/src/libeai/EAI_C_Field.o 
rm 	../freex3d/src/libeai/EAI_C_Advise.o 
rm 	../freex3d/src/libeai/EAI_C_Internals.o 
rm 	../freex3d/src/libeai/EAI_C_Node.o 
rm 	../freex3d/src/libeai/EAI_C_ReWire.o 
rm 	../freex3d/src/libeai/GeneratedCode.o 
lipo -create libEAI.i386 libEAI.ppc -output libEAI.dylib

rm libEAI.i386
rm libEAI.ppc

install_name_tool -id libEAI.dylib libEAI.dylib
cp libEAI.dylib /usr/local/lib

rm libEAI.dylib
