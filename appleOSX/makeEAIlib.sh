# brute force compile the eailib.dylib on OSX
#

###############
# PowerPC 
###############


gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch ppc -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.3.9.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_Control.c -o ../src/libeai/EAI_C_Control.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch ppc -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.3.9.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_Field.c -o ../src/libeai/EAI_C_Field.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch ppc -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.3.9.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_Advise.c -o ../src/libeai/EAI_C_Advise.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch ppc -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.3.9.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_Internals.c -o ../src/libeai/EAI_C_Internals.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch ppc -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.3.9.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_Node.c -o ../src/libeai/EAI_C_Node.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch ppc -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.3.9.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_ReWire.c -o ../src/libeai/EAI_C_ReWire.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch ppc -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.3.9.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/GeneratedCode.c -o ../src/libeai/GeneratedCode.o 

gcc -dynamiclib  \
	-arch ppc \
	-L/usr/local/lib -lFreeWRLFunc \
	../src/libeai/EAI_C_Control.o \
	../src/libeai/EAI_C_Field.o \
	../src/libeai/EAI_C_Advise.o \
	../src/libeai/EAI_C_Internals.o \
	../src/libeai/EAI_C_Node.o \
	../src/libeai/EAI_C_ReWire.o \
	../src/libeai/GeneratedCode.o \
	-o libEAI.ppc

rm 	../src/libeai/EAI_C_Control.o 
rm 	../src/libeai/EAI_C_Field.o 
rm 	../src/libeai/EAI_C_Advise.o 
rm 	../src/libeai/EAI_C_Internals.o 
rm 	../src/libeai/EAI_C_Node.o 
rm 	../src/libeai/EAI_C_ReWire.o 
rm 	../src/libeai/GeneratedCode.o 


###############
# 32 bit Intel
###############


gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch i386 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_Control.c -o ../src/libeai/EAI_C_Control.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch i386 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_Field.c -o ../src/libeai/EAI_C_Field.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch i386 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_Advise.c -o ../src/libeai/EAI_C_Advise.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch i386 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
	-IOSX_Specific \
	-I../src/lib/input \
	-I../src/lib \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_Internals.c -o ../src/libeai/EAI_C_Internals.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch i386 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_Node.c -o ../src/libeai/EAI_C_Node.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch i386 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_ReWire.c -o ../src/libeai/EAI_C_ReWire.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch i386 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/GeneratedCode.c -o ../src/libeai/GeneratedCode.o 

gcc -dynamiclib  \
	-L/usr/local/lib -lFreeWRLFunc \
	../src/libeai/EAI_C_Control.o \
	../src/libeai/EAI_C_Field.o \
	../src/libeai/EAI_C_Advise.o \
	../src/libeai/EAI_C_Internals.o \
	../src/libeai/EAI_C_Node.o \
	../src/libeai/EAI_C_ReWire.o \
	../src/libeai/GeneratedCode.o \
	-o libEAI.i386


rm 	../src/libeai/EAI_C_Control.o 
rm 	../src/libeai/EAI_C_Field.o 
rm 	../src/libeai/EAI_C_Advise.o 
rm 	../src/libeai/EAI_C_Internals.o 
rm 	../src/libeai/EAI_C_Node.o 
rm 	../src/libeai/EAI_C_ReWire.o 
rm 	../src/libeai/GeneratedCode.o 

###############
# 64 bit Intel
###############


gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch x86_64 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
	-I../opt/local/include \
        -I../src/libeai \
	../src/libeai/EAI_C_Control.c -o ../src/libeai/EAI_C_Control.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch x86_64 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_Field.c -o ../src/libeai/EAI_C_Field.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch x86_64 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_Advise.c -o ../src/libeai/EAI_C_Advise.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch x86_64 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
	-IOSX_Specific \
	-I../src/lib/input \
	-I../src/lib \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_Internals.c -o ../src/libeai/EAI_C_Internals.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch x86_64 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_Node.c -o ../src/libeai/EAI_C_Node.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch x86_64 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/EAI_C_ReWire.c -o ../src/libeai/EAI_C_ReWire.o 
gcc -c \
	-DREWIRE \
	-DTARGET_AQUA \
	-D_REENTRANT -DAQUA \
	-fno-common \
	-arch x86_64 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
	-IOSX_Specific \
	-I../src/lib \
	-I../src/lib/input \
        -I../src/libeai \
	-I../opt/local/include \
	../src/libeai/GeneratedCode.c -o ../src/libeai/GeneratedCode.o 

gcc -dynamiclib  \
	-L/usr/local/lib -lFreeWRLFunc \
	../src/libeai/EAI_C_Control.o \
	../src/libeai/EAI_C_Field.o \
	../src/libeai/EAI_C_Advise.o \
	../src/libeai/EAI_C_Internals.o \
	../src/libeai/EAI_C_Node.o \
	../src/libeai/EAI_C_ReWire.o \
	../src/libeai/GeneratedCode.o \
	-o libEAI.x86_64

rm 	../src/libeai/EAI_C_Control.o 
rm 	../src/libeai/EAI_C_Field.o 
rm 	../src/libeai/EAI_C_Advise.o 
rm 	../src/libeai/EAI_C_Internals.o 
rm 	../src/libeai/EAI_C_Node.o 
rm 	../src/libeai/EAI_C_ReWire.o 
rm 	../src/libeai/GeneratedCode.o 

lipo -create libEAI.x86_64 libEAI.i386 libEAI.ppc -output libEAI.dylib
#lipo -create libEAI.x86_64 libEAI.i386  -output libEAI.dylib
#lipo -create libEAI.x86_64 -output libFreeWRLEAI.dylib

rm libEAI.x86_64
rm libEAI.i386
rm libEAI.ppc

install_name_tool -id libFreeWRLEAI.dylib libFreeWRLEAI.dylib
cp libFreeWRLEAI.dylib /usr/local/lib

rm libFreeWRLEAI.dylib
