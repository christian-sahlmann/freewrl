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
component=freewrl_plugin
PLUGIN_VERSION=`cat $$_EV`

BUILD_TIMESTAMP=`date +%F_%T`
AM_CPPFLAGS += -DBUILD_TIMESTAMP=\"$(BUILD_TIMESTAMP)\"

# Generate the version source file
internal_version.c: $(_EV)
	$(top_srcdir)/vtempl $(component) \
	$(PLUGIN_VERSION) < $(templ) > $@
# And add it to the list of files to clean
CLEANFILES=internal_version.c

newversion:
	-rm -f internal_version.*
	$(MAKE)

LIBFREEWRL_SRCDIR=$(top_srcdir)/src/lib
LIBFREEWRL_BUILDDIR=$(top_builddir)/src/lib

AM_CPPFLAGS += -I$(LIBFREEWRL_SRCDIR)

# libtool does not help much here, I'll end using gcc -shared
# if we don't find a smart way to instruct libtool...

plugin_LTLIBRARIES = libFreeWRLplugin.la
libFreeWRLplugin_la_LIBTOOLFLAGS = --tag=disable-static

# This makes the plugin installs in /usr/lib/mozilla/plugins...
# we do not want this for "make distcheck" to pass...
# plugindir=$(PLUGIN_DIR)
# We want plugin installed along libFreeWRL and then
# if we have the rights to, create a link from /usr/lib/mozilla/plugins
# to the plugin.
plugindir=$(libdir)
libFreeWRLplugin_la_LDFLAGS = -module -avoid-version $(AM_LDFLAGS)

include Makefile.sources

libFreeWRLplugin_la_SOURCES += internal_version.c

register-plugin:
	mkdir -p $(DESTDIR)$(libdir)
	mkdir -p $(DESTDIR)$(PLUGIN_DIR)
	if test -w $(DESTDIR)$(PLUGIN_DIR); then \
		cd $(DESTDIR)$(PLUGIN_DIR); \
		ln -s $(libdir)/$(FREEWRL_PLUGIN); \
	fi

install-exec-am: register-plugin
