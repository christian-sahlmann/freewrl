# Helper program: FreeWRL sound server
#
#

# Here we can use variable checked and defined by toplevel configure
# AM_CPPFLAGS = \
# 				$(PTHREAD_CFLAGS) \
# 				$(X_CFLAGS) \
# 				$(MOTIF_CFLAGS) \
# 				$(GL_CFLAGS)

# AM_LDFLAGS = \
# 				$(FREETYPE_LIBS) \
# 				$(JAVASCRIPT_ENGINE_LIBS) \
# 				$(XML_LIBS) $(EXPAT_LIBS) \
# 				$(X_PRE_LIBS) $(X_LIBS) $(X_EXTRA_LIBS) \
# 				$(MOTIF_LIBS) \
# 				$(GL_LIBS) \
# 				$(JPEG_LIBS) \
# 				$(PNG_LIBS)


_EV=${top_srcdir}/versions/SOUNDMGR
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

bin_PROGRAMS = freewrl_snd

include .SOURCES

freewrl_snd_SOURCES += internal_version.c

