# Main program: training
# Uses libtraining in ../lib

# Here we can use variable checked and defined by toplevel configure
AM_CPPFLAGS = \
				$(MOZILLA_PLUGIN_CFLAGS)
				$(X_CFLAGS)

AM_LDFLAGS = \
				$(MOZILLA_PLUGIN_LIBS) \
				$(X_PRE_LIBS) $(X_LIBS) $(X_EXTRA_LIBS)

_EV=${top_srcdir}/versions/PLUGIN
export

templ=${top_srcdir}/versions/template/version.c.in
component=freex3d_plugin
PLUGIN_VERSION=`cat $$_EV`

# Generate the version source file
internal_version.c: $(_EV)
	${top_srcdir}/vtempl $(component) \
	$(PLUGIN_VERSION) < $(templ) > $@
# And add it to the list of files to clean
CLEANFILES=internal_version.c

LIBFREEWRL_SRCDIR=$(top_srcdir)/src/lib
LIBFREEWRL_BUILDDIR=$(top_builddir)/src/lib

AM_CPPFLAGS += -I$(LIBFREEWRL_SRCDIR)

lib_LTLIBRARIES = libFreeX3Dplugin.la
libFreeX3Dplugin_la_LDFLAGS = -module -avoid-version
libFreeX3Dplugin_la_LIBADD = $(LIBFREEWRL_BUILDDIR)/libFreeX3D.la

include Makefile.sources

libFreeX3Dplugin_la_SOURCES += internal_version.c
