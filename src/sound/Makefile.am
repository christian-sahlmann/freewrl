# Helper program: FreeX3D sound server
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
component=freex3d_snd
FREEX3D_SND_VERSION=`cat $$_EV`

# Generate the version source file
internal_version.c: $(_EV)
	$(top_srcdir)/vtempl $(component) \
	$(FREEX3D_SND_VERSION) < $(templ) > $@
# And add it to the list of files to clean
CLEANFILES=internal_version.c

bin_PROGRAMS = freex3d_snd

include Makefile.sources

freex3d_snd_SOURCES += internal_version.c

