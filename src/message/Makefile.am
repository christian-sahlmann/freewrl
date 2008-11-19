# Helper program: FreeX3D message server
#
#

# Get global variables for Makefile defined by toplevel Makefile.globals
include $(top_srcdir)/Makefile.globals

# Just add variables tested and checked by troplevel configure
AM_CPPFLAGS += \
				$(X_CFLAGS) \
				$(XAW_CFLAGS) \
				$(MOTIF_CFLAGS)

AM_LDFLAGS += \
				$(X_PRE_LIBS) $(X_LIBS) $(X_EXTRA_LIBS) \
				$(XAW_LIBS) \
				$(MOTIF_LIBS)

_EV=$(top_srcdir)/versions/MESSAGEMGR
export

templ=$(top_srcdir)/versions/template/version.c.in
component=freex3d_snd
FREEWRL_VERSION=`cat $$_EV`

# Generate the version source file
internal_version.c: $(_EV)
	$(top_srcdir)/vtempl $(component) \
	$(FREEWRL_VERSION) < $(templ) > $@
# And add it to the list of files to clean
CLEANFILES=internal_version.c

bin_PROGRAMS = freex3d_msg

include Makefile.sources

freex3d_msg_SOURCES += internal_version.c

