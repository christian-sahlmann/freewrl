# Helper program: FreeWRL sound server
#
#

# Get global variables for Makefile defined by toplevel Makefile.globals
include $(top_srcdir)/Makefile.globals

# Just add variables tested and checked by troplevel configure
##AM_CPPFLAGS += \
##AM_LDFLAGS += \

# Embedded versioning
_EV=$(top_srcdir)/versions/SOUNDMGR
export

templ=$(top_srcdir)/versions/template/version.c.in
component=freewrl_snd
FREEWRL_SND_VERSION=`cat $$_EV`

# Generate the version source file
internal_version.c: $(_EV)
	$(top_srcdir)/vtempl $(component) \
	$(FREEWRL_SND_VERSION) < $(templ) > $@
# And add it to the list of files to clean
CLEANFILES=internal_version.c

# Compile/Link with FreeWRL library
LIBFREEWRL_SRCDIR=$(top_srcdir)/src/lib
LIBFREEWRL_BUILDDIR=$(top_builddir)/src/lib
AM_CPPFLAGS += -I$(LIBFREEWRL_SRCDIR)

bin_PROGRAMS = freewrl_snd
freewrl_snd_LDADD = $(LIBFREEWRL_BUILDDIR)/libFreeWRL.la

include Makefile.sources

freewrl_snd_SOURCES += internal_version.c

