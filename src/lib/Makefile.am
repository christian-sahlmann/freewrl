# Library: main FreeWRL library
# Will generates its own pkg-config definition file (soon).
#

# Get global variables for Makefile defined by toplevel Makefile.globals
include $(top_srcdir)/Makefile.globals

# Just add variables tested and checked by toplevel configure

# General libraries used with all platforms
AM_CPPFLAGS += \
				$(PTHREAD_CFLAGS) \
				$(FREETYPE_CFLAGS) \
				$(JAVASCRIPT_ENGINE_CFLAGS) \
				$(XML_CFLAGS) $(EXPAT_CFLAGS) \
				$(GL_CFLAGS) \
				$(PNG_CFLAGS)

EXTERNAL_LIBS = \
				$(PTHREAD_LIBS) \
				$(FREETYPE_LIBS) \
				$(JAVASCRIPT_ENGINE_LIBS) \
				$(XML_LIBS) $(EXPAT_LIBS) \
				$(GL_LIBS) \
				$(JPEG_LIBS) \
				$(PNG_LIBS)


# X11 flags specific to x11 target (default on Linux -- optional on Mac)
if WINDOW_USE_X11

AM_CPPFLAGS += \
				$(X_CFLAGS) \
				$(XAW_CFLAGS)
EXTERNAL_LIBS += \
				$(X_PRE_LIBS) $(X_LIBS) $(X_EXTRA_LIBS) \
				$(XAW_LIBS)
endif

# Motif flags specific to motif target (only on Linux at the moment)
if WINDOW_USE_MOTIF

AM_CPPFLAGS += \
				$(X_CFLAGS) \
				$(XAW_CFLAGS) \
				$(MOTIF_CFLAGS)
EXTERNAL_LIBS += \
				$(X_PRE_LIBS) $(X_LIBS) $(X_EXTRA_LIBS) \
				$(XAW_LIBS) \
				$(MOTIF_LIBS)
endif

# Aqua flags specific to aqua target (only on Mac)
if WINDOW_USE_AQUA

AM_CPPFLAGS += $(MAC_CFLAGS) -DAQUA

endif

if OS_MAC

# I found this in spidermonkey build files: -Wall -Wno-format -no-cpp-precomp -fno-common -DJS_THREADSAFE -DXP_UNIX -DSVR4 -DSYSV -D_BSD_SOURCE -DPOSIX_SOURCE -DDARWIN  -UDEBUG -DNDEBUG -UDEBUG_root -DJS_THREADSAFE -DEDITLINE

AM_CPPFLAGS += -Wno-format -no-cpp-precomp -fno-common 

endif

## the freewrl code uses a lot of type-punned pointers; turning off strict-aliasing
AM_CFLAGS += -fno-strict-aliasing


_EV=$(top_srcdir)/versions/LIBFREEWRL
_EV2=$(top_srcdir)/versions/LIBFREEWRL_LTVERSION
export

templ=$(top_srcdir)/versions/template/version.c.in
component=libFreeWRL
LIBFREEWRL_VERSION=`cat $$_EV`
LIBFREEWRL_LTVERSION=`cat $$_EV2`

# Generate the version source file
internal_version.c: $(_EV) $(_EV2)
	$(top_srcdir)/vtempl $(component) \
	$(LIBFREEWRL_VERSION) < $(templ) > $@
# And add it to the list of files to clean
CLEANFILES=internal_version.c

newversion:
	-rm -f internal_version.*
	$(MAKE)

# To be able to check for libFreeWRL with pkg-config
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libFreeWRL.pc

lib_LTLIBRARIES = libFreeWRL.la
include_HEADERS = libFreeWRL.h
libFreeWRL_la_LDFLAGS = -version-info $(LIBFREEWRL_LTVERSION)
libFreeWRL_la_LIBADD = $(EXTERNAL_LIBS)

include Makefile.sources

libFreeWRL_la_SOURCES += internal_version.c

#
# WINDOW_USE_* are exclusive
#
if WINDOW_USE_X11
libFreeWRL_la_SOURCES += display_x11.c ui/fwWindow.c ui/fwWindow.h ui/fwBareWindow.c ui/fwBareWindow.h
AM_CPPFLAGS+=-DTARGET_X11
endif

if WINDOW_USE_MOTIF
libFreeWRL_la_SOURCES += display_x11.c display_motif.c ui/fwWindow.c ui/fwWindow.h ui/fwMotifWindow.c ui/fwMotifWindow.h
AM_CPPFLAGS+=-DTARGET_MOTIF
endif

if WINDOW_USE_AQUA
libFreeWRL_la_SOURCES += display_aqua.c
AM_CPPFLAGS+=-DTARGET_AQUA
endif
