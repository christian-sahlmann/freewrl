# Library: main FreeX3D library
# Will generates its own pkg-config definition file (soon).
#

# Here we can use variable checked and defined by toplevel configure
AM_CPPFLAGS = \
				$(PTHREAD_CFLAGS) \
				$(FREETYPE_CFLAGS) \
				$(JAVASCRIPT_ENGINE_CFLAGS) \
				$(XML_CFLAGS) $(EXPAT_CFLAGS) \
				$(X_CFLAGS) \
				$(XAW_CFLAGS) \
				$(MOTIF_CFLAGS) \
				$(GL_CFLAGS) \
				$(PNG_CFLAGS)

if DEBUG
AM_CPPFLAGS+=-g -DDEBUG
endif

AM_LDFLAGS = \
				$(FREETYPE_LIBS) \
				$(JAVASCRIPT_ENGINE_LIBS) \
				$(XML_LIBS) $(EXPAT_LIBS) \
				$(X_PRE_LIBS) $(X_LIBS) $(X_EXTRA_LIBS) \
				$(XAW_LIBS) \
				$(MOTIF_LIBS) \
				$(GL_LIBS) \
				$(JPEG_LIBS) \
				$(PNG_LIBS)

_EV=${top_srcdir}/versions/LIBFREEX3D
_EV2=${top_srcdir}/versions/LIBFREEX3D_LTVERSION
export

templ=${top_srcdir}/versions/template/version.c.in
component=libFreeX3D
LIBFREEX3D_VERSION=`cat $$_EV`
LIBFREEX3D_LTVERSION=`cat $$_EV2`

# Generate the version source file
internal_version.c: $(_EV)
	${top_srcdir}/vtempl $(component) \
	$(LIBFREEX3D_VERSION) < $(templ) > $@
# And add it to the list of files to clean
CLEANFILES=internal_version.c

# To be able to check for libFreeX3D with pkg-config
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libFreeX3D.pc

lib_LTLIBRARIES = libFreeX3D.la
include_HEADERS = libFreeX3D.h
libFreeX3D_la_LDFLAGS = -version-info $(LIBFREEX3D_LTVERSION)

include Makefile.sources

libFreeX3D_la_SOURCES += internal_version.c

#
# WINDOW_USE_* are exclusive
#
if WINDOW_USE_X11
libFreeX3D_la_SOURCES += display_x11.c
AM_CPPFLAGS+=-DTARGET_X11
endif

if WINDOW_USE_MOTIF
libFreeX3D_la_SOURCES += display_motif.c
AM_CPPFLAGS+=-DTARGET_MOTIF
endif

if WINDOW_USE_AQUA
libFreeX3D_la_SOURCES += display_aqua.c
AM_CPPFLAGS+=-DTARGET_AQUA
endif