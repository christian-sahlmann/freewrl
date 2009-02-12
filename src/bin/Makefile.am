# Main program: FreeWRL program
# Uses libFreeWRL in ../lib
#

# Get global variables for Makefile defined by toplevel Makefile.globals
include $(top_srcdir)/Makefile.globals

# Just add variables tested and checked by troplevel configure
AM_CPPFLAGS += \
				$(PTHREAD_CFLAGS) \
				$(JAVASCRIPT_ENGINE_CFLAGS) \
				$(X_CFLAGS) \
				$(XAW_CFLAGS) \
				$(MOTIF_CFLAGS) \
				$(GL_CFLAGS)

AM_LDFLAGS += \
				$(PTHREAD_LIBS) \
				$(FREETYPE_LIBS) \
				$(JAVASCRIPT_ENGINE_LIBS) \
				$(XML_LIBS) $(EXPAT_LIBS) \
				$(X_PRE_LIBS) $(X_LIBS) $(X_EXTRA_LIBS) \
				$(XAW_LIBS) \
				$(MOTIF_LIBS) \
				$(GL_LIBS) \
				$(JPEG_LIBS) \
				$(PNG_LIBS)


_EV=$(top_srcdir)/versions/FREEWRL
export

templ=$(top_srcdir)/versions/template/version.c.in
component=freewrl
FREEWRL_VERSION=`cat $$_EV`

# Generate the version source file
internal_version.c: $(_EV)
	$(top_srcdir)/vtempl $(component) \
	$(FREEWRL_VERSION) < $(templ) > $@
# And add it to the list of files to clean
CLEANFILES=internal_version.c

LIBFREEWRL_SRCDIR=$(top_srcdir)/src/lib
LIBFREEWRL_BUILDDIR=$(top_builddir)/src/lib

AM_CPPFLAGS += -I$(LIBFREEWRL_SRCDIR)

bin_PROGRAMS = freewrl
freewrl_LDADD = $(LIBFREEWRL_BUILDDIR)/libFreeWRL.la

include Makefile.sources

freewrl_SOURCES += internal_version.c
