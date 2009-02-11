# Toplevel Makefile.am
#
# $Id: Makefile.am,v 1.6 2009/02/11 15:12:54 istakenv Exp $
#
# Defines build subdirectories.
# Defines all files distributed by this package.

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
