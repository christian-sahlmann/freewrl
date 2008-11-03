# Main program: FreeX3D program
# Uses libFreeX3D in ../lib
#

# Here we can use variable checked and defined by toplevel configure
AM_CPPFLAGS = \
				$(PTHREAD_CFLAGS) \
				$(JAVASCRIPT_ENGINE_CFLAGS) \
				$(X_CFLAGS) \
				$(XAW_CFLAGS) \
				$(MOTIF_CFLAGS) \
				$(GL_CFLAGS)

AM_LDFLAGS = \
				$(FREETYPE_LIBS) \
				$(JAVASCRIPT_ENGINE_LIBS) \
				$(XML_LIBS) $(EXPAT_LIBS) \
				$(X_PRE_LIBS) $(X_LIBS) $(X_EXTRA_LIBS) \
				$(XAW_LIBS) \
				$(MOTIF_LIBS) \
				$(GL_LIBS) \
				$(JPEG_LIBS) \
				$(PNG_LIBS)


_EV=${top_srcdir}/versions/FREEX3D
export

templ=${top_srcdir}/versions/template/version.c.in
component=freex3d
FREEX3D_VERSION=`cat $$_EV`

# Generate the version source file
internal_version.c: $(_EV)
	${top_srcdir}/vtempl $(component) \
	$(FREEX3D_VERSION) < $(templ) > $@
# And add it to the list of files to clean
CLEANFILES=internal_version.c

LIBFREEX3D_SRCDIR=$(top_srcdir)/src/lib
LIBFREEX3D_BUILDDIR=$(top_builddir)/src/lib

AM_CPPFLAGS += -I$(LIBFREEX3D_SRCDIR)

bin_PROGRAMS = freex3d
freex3d_LDADD = $(LIBFREEX3D_BUILDDIR)/libFreeX3D.la

include Makefile.sources

freex3d_SOURCES += internal_version.c
