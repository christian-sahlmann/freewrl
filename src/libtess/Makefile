# libtess/Makefile

TESS_LIB_NAME = libtess.a

C_SOURCES = \
	dict.c		\
	geom.c		\
	memalloc.c	\
	mesh.c		\
	normal.c	\
	priorityq.c	\
	render.c	\
	sweep.c		\
	tess.c		\
	tessmono.c

SOURCES = $(C_SOURCES)

C_OBJECTS = $(C_SOURCES:.c=.o)
OBJECTS = $(C_OBJECTS) 


##### RULES #####

.c.o:
	$(CC) -c $(CFLAGS) -DNDEBUG  $< -o $@

##### TARGETS #####

default:
	$(MAKE) $(TESS_LIB_NAME) || exit 1 ; \

# Make the library:
$(TESS_LIB_NAME): $(TESS_LIB_NAME)($(OBJECTS))


clean:
	-rm -f *.o

