# Helper program: FreeWRL message server
#
#

# Here we can use variable checked and defined by toplevel configure
AM_CPPFLAGS = \
				$(X_CFLAGS) \
				$(XAW_CFLAGS) \
				$(MOTIF_CFLAGS)

AM_LDFLAGS = \
				$(X_PRE_LIBS) $(X_LIBS) $(X_EXTRA_LIBS) \
				$(XAW_LIBS) \
				$(MOTIF_LIBS)

_EV=${top_srcdir}/versions/MESSAGEMGR
export

templ=${top_srcdir}/versions/template/version.c.in
component=freewrl_snd
FREEWRL_VERSION=`cat $$_EV`

# Generate the version source file
internal_version.c: $(_EV)
	${top_srcdir}/vtempl $(component) \
	$(FREEWRL_VERSION) < $(templ) > $@
# And add it to the list of files to clean
CLEANFILES=internal_version.c

# LIBFREEWRL_SRCDIR=$(top_srcdir)/src/lib
# LIBFREEWRL_BUILDDIR=$(top_builddir)/src/lib

# AM_CPPFLAGS += -I$(LIBFREEWRL_SRCDIR)

bin_PROGRAMS = freewrl_msg

include .SOURCES

freewrl_msg_SOURCES += internal_version.c

