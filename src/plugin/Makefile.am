# Main program: training
# Uses libtraining in ../lib


# Get global variables for Makefile defined by toplevel Makefile.globals
include $(top_srcdir)/Makefile.globals

# Just add variables tested and checked by troplevel configure

# General libraries used with all platforms
AM_CPPFLAGS += \
				$(PLUGIN_CFLAGS)

AM_LDFLAGS += \
				$(PLUGIN_LIBS)


# X11 flags specific to x11 target (default on Linux -- optional on Mac)
if WINDOW_USE_AQUA
ERR=Error: Build system does not currently support Mac Safari Target
echo $(ERR)
else
AM_CPPFLAGS += \
				$(X_CFLAGS) 
endif

_EV=$(top_srcdir)/versions/PLUGIN
export

templ=$(top_srcdir)/versions/template/version.c.in
component=freex3d_plugin
PLUGIN_VERSION=`cat $$_EV`

# Generate the version source file
internal_version.c: $(_EV)
	$(top_srcdir)/vtempl $(component) \
	$(PLUGIN_VERSION) < $(templ) > $@
# And add it to the list of files to clean
CLEANFILES=internal_version.c

LIBFREEWRL_SRCDIR=$(top_srcdir)/src/lib
LIBFREEWRL_BUILDDIR=$(top_builddir)/src/lib

AM_CPPFLAGS += -I$(LIBFREEWRL_SRCDIR)

plugin_LTLIBRARIES = libFreeX3Dplugin.la
plugindir=$(PLUGIN_DIR)
libFreeX3Dplugin_la_LDFLAGS = -avoid-version $(AM_LDFLAGS)
# libFreeX3Dplugin_la_LIBADD = $(LIBFREEWRL_BUILDDIR)/libFreeX3D.la

include Makefile.sources

libFreeX3Dplugin_la_SOURCES += internal_version.c
