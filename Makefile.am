# Toplevel Makefile.am
#
# $Id: Makefile.am,v 1.9 2009/10/26 08:03:34 couannette Exp $
#
# Defines build subdirectories.
# Defines all files distributed by this package.

# Get global variables for Makefile defined by toplevel Makefile.globals
include $(top_srcdir)/Makefile.globals

# See configure.ac
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src

EXTRA_DIST = ChangeLog \
README \
AUTHORS \
autogen.sh \
COPYING \
INSTALL \
NEWS \
vtempl.in \
Makefile.globals \
spidermonkey.pc \
data \
doc/AUTOCONF \
doc/AUTOMAKE \
doc/DEPENDENCY \
doc/LIBTOOL \
doc/VERSION \
versions/FREEWRL \
versions/JAVACLASSES \
versions/LIBFREEWRL \
versions/LIBFREEWRL_LTVERSION \
versions/MESSAGEMGR \
versions/PLUGIN \
versions/SOUNDMGR \
versions/template/version.c.in

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = data/freewrl.png

dist_man_MANS = doc/freewrl.1

# Since data is included in EXTRA_DIR, all this subdir
# will be copied into the dist, so we must clean up the CVS files...
dist-hook:
	-rm -rf $(distdir)/data/CVS

install-data-local: $(top_srcdir)/data/freewrl.desktop
	mkdir -p $(DESTDIR)$(datadir)/applications
	install $(top_srcdir)/data/freewrl.desktop $(DESTDIR)$(datadir)/applications

dist-minimal: clean distclean
	if test -f freewrl-$(PACKAGE_VERSION).tar.gz; then \
		rm -f freewrl-$(PACKAGE_VERSION).tar.gz; fi
	find . -type f -name "*~" -exec rm {} \;
	rm -rf aclocal.m4 autom4te.cache config config.h.in configure $$(find . -type f -name Makefile.in | xargs)
	cd .. && \
	if test -h freewrl-$(PACKAGE_VERSION); then \
		rm -f freewrl-$(PACKAGE_VERSION); fi && \
	ln -s freewrl freewrl-$(PACKAGE_VERSION) && \
	tar cfz freewrl-minimal-$(PACKAGE_VERSION).tar.gz \
		freewrl-$(PACKAGE_VERSION)/* --exclude '.svn'

all: vtempl
