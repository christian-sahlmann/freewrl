# Main program: training
# Uses libtraining in ../lib


# Get global variables for Makefile defined by toplevel Makefile.globals
include $(top_srcdir)/Makefile.globals

# Just add variables tested and checked by troplevel configure

# General libraries used with all platforms
AM_CPPFLAGS += \
                                $(PTHREAD_CFLAGS) \
                                $(FREETYPE_CFLAGS) \
                                $(JAVASCRIPT_ENGINE_CFLAGS) \
                                $(XML_CFLAGS) $(EXPAT_CFLAGS) \
                                $(GL_CFLAGS) \
                                $(PNG_CFLAGS)

AM_LDFLAGS += \
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
AM_LDFLAGS += \
                                $(X_PRE_LIBS) $(X_LIBS) $(X_EXTRA_LIBS) \
                                $(XAW_LIBS)
endif

# Motif flags specific to motif target (only on Linux at the moment)
if WINDOW_USE_MOTIF
AM_CPPFLAGS += \
                                $(X_CFLAGS) \
                                $(XAW_CFLAGS) \
                                $(MOTIF_CFLAGS)
AM_LDFLAGS += \
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
### -DSVR4 -D_BSD_SOURCE -DPOSIX_SOURCE -D_POSIX_C_SOURCE -DDARWIN
endif


_EV=$(top_srcdir)/versions/LIBEAI
_EV2=$(top_srcdir)/versions/LIBEAI_LTVERSION
export

templ=$(top_srcdir)/versions/template/version.c.in
component=libEAI
LIBEAI_VERSION=`cat $$_EV`
LIBEAI_LTVERSION=`cat $$_EV2`

BUILD_TIMESTAMP=`date +%F_%T`
AM_CPPFLAGS += -DBUILD_TIMESTAMP=\"$(BUILD_TIMESTAMP)\"

# Generate the version source file
internal_version.c: $(_EV)
	$(top_srcdir)/vtempl $(component) \
	$(LIBEAI_VERSION) < $(templ) > $@
# And add it to the list of files to clean
CLEANFILES=internal_version.c

LIBFREEWRL_SRCDIR=$(top_srcdir)/src/lib
LIBFREEWRL_BUILDDIR=$(top_builddir)/src/lib

# To be able to check for libEAI with pkg-config
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libEAI.pc


AM_CPPFLAGS += -I$(LIBFREEWRL_SRCDIR)

lib_LTLIBRARIES = libEAI.la
include_HEADERS = EAI_C.h
libEAI_la_LDFLAGS = -version-info $(LIBEAI_LTVERSION) $(AM_LDFLAGS)
# libEAI_la_LIBADD = $(LIBFREEWRL_BUILDDIR)/libFreeWRL.la

include Makefile.sources

libEAI_la_SOURCES += internal_version.c
