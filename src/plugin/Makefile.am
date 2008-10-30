# Main program: training
# Uses libtraining in ../lib

# Here we can use variable checked and defined by toplevel configure
AM_CPPFLAGS = \
				$(MOZILLA_PLUGIN_CFLAGS)
				$(X_CFLAGS)

AM_LDFLAGS = \
				$(MOZILLA_PLUGIN_LIBS) \
				$(X_PRE_LIBS) $(X_LIBS) $(X_EXTRA_LIBS) \
				-shared

_EV=${top_srcdir}/versions/PLUGIN
export

templ=${top_srcdir}/versions/template/version.c.in
component=freewrl_plugin
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

lib_LTLIBRARIES = libfreewrlplugin.la
libfreewrlplugin_la_LDFLAGS = -module -avoid-version
libfreewrlplugin_la_LIBADD = $(LIBFREEWRL_BUILDDIR)/libFreeWRL.la

include .SOURCES

libfreewrlplugin_la_SOURCES += internal_version.c
