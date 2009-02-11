#Make file for FreeWRL
CC = gcc
JAVAC = javac
INC = -I/usr/include -I/usr/include -I/usr/local/include -I../lib  -I. 
DEFINE = -fno-stack-protector 
FWFLAGS =  -g -fPIC
MYLIBS = -L/usr/lib -L/usr/local/lib 
MYFELIBS = -L. -lpthread  -L/usr/lib -L/usr/local/lib -lFreeX3D 
MYSELIBS = -lm 
MYMESLIBS = -lXaw 
FRONTEND_DIR = /usr/bin
LIB_DIR = /usr/lib

CFLAGS = $(FWFLAGS) $(INC) $(DEFINE)


#EAIlib Make
LIBEAISRCS = EAI_C_Advise.c EAI_C_Control.c EAI_C_Field.c EAI_C_Internals.c EAI_C_Node.c EAI_C_ReWire.c GeneratedCode.c

LIBEAIOBJS= EAI_C_Advise.o EAI_C_Control.o EAI_C_Field.o EAI_C_Internals.o EAI_C_Node.o EAI_C_ReWire.o GeneratedCode.o

all: libEAI 

libEAI:		$(LIBEAIOBJS)
	$(CC) -shared $(MYFELIBS) -o libEAI.so $(LIBEAIOBJS)

clean:
	rm -rf ../*.o
	rm -rf /usr/local/lib/libEAI.so

install:	all	
	cp -f libEAI.so /usr/local/lib/libEAI.so
