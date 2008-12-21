# Library: main FreeX3D library
# Will generates its own pkg-config definition file (soon).
#

# Get global variables for Makefile defined by toplevel Makefile.globals
include $(top_srcdir)/Makefile.globals

# Just add variables tested and checked by troplevel configure
AM_CPPFLAGS += \
				$(PTHREAD_CFLAGS) \
				$(FREETYPE_CFLAGS) \
				$(JAVASCRIPT_ENGINE_CFLAGS) \
				$(XML_CFLAGS) $(EXPAT_CFLAGS) \
				$(GL_CFLAGS) \
				$(PNG_CFLAGS)

if WINDOW_USE_X11
AM_CPPFLAGS += \
				$(X_CFLAGS) \
				$(XAW_CFLAGS)
endif

if WINDOW_USE_MOTIF
AM_CPPFLAGS += \
				$(X_CFLAGS) \
				$(XAW_CFLAGS) \
				$(MOTIF_CFLAGS)
endif

AM_LDFLAGS += \
				$(FREETYPE_LIBS) \
				$(JAVASCRIPT_ENGINE_LIBS) \
				$(XML_LIBS) $(EXPAT_LIBS) \
				$(GL_LIBS) \
				$(JPEG_LIBS) \
				$(PNG_LIBS)

if WINDOW_USE_X11
AM_LDFLAGS += \
				$(X_PRE_LIBS) $(X_LIBS) $(X_EXTRA_LIBS) \
				$(XAW_LIBS)
endif

if WINDOW_USE_MOTIF
AM_LDFLAGS += \
				$(X_PRE_LIBS) $(X_LIBS) $(X_EXTRA_LIBS) \
				$(XAW_LIBS) \
				$(MOTIF_LIBS)
endif


_EV=$(top_srcdir)/versions/LIBFREEX3D
_EV2=$(top_srcdir)/versions/LIBFREEX3D_LTVERSION
export

templ=$(top_srcdir)/versions/template/version.c.in
component=libFreeX3D
LIBFREEX3D_VERSION=`cat $$_EV`
LIBFREEX3D_LTVERSION=`cat $$_EV2`

# Generate the version source file
internal_version.c: $(_EV)
	$(top_srcdir)/vtempl $(component) \
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
libFreeX3D_la_SOURCES += display_x11.c ui/fwBareWindow.c
AM_CPPFLAGS+=-DTARGET_X11
endif

if WINDOW_USE_MOTIF
libFreeX3D_la_SOURCES += display_x11.c display_motif.c ui/fwMotifWindow.c
AM_CPPFLAGS+=-DTARGET_MOTIF
endif

if WINDOW_USE_AQUA
libFreeX3D_la_SOURCES += display_aqua.c
AM_CPPFLAGS+=-DTARGET_AQUA
endif

# # quickcompile:
# # 	@-rm -f .quickcompile.log
# # 	@for i in $(filter %.c, $(libFreeX3D_la_SOURCES)); do \
# # 		echo "compile: $$i"; \
# # 		$(COMPILE) $(CFLAGS) -c $$i 2>&1 >> .quickcompile.log || { \
# # 			echo "failed to compile: $$i"; \
# # 			exit 1; \
# # 		}; \
# # 	done
