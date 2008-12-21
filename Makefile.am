# Toplevel Makefile.am
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
versions/FREEX3D \
versions/JAVACLASSES \
versions/LIBFREEX3D \
versions/LIBFREEX3D_LTVERSION \
versions/MESSAGEMGR \
versions/PLUGIN \
versions/SOUNDMGR \
versions/template/version.c.in

dist-minimal: clean distclean
	if test -f freex3d-$(PACKAGE_VERSION).tar.gz; then \
		rm -f freex3d-$(PACKAGE_VERSION).tar.gz; fi
	find . -type f -name "*~" -exec rm {} \;
	rm -rf aclocal.m4 autom4te.cache config config.h.in configure $$(find . -type f -name Makefile.in | xargs)
	cd .. && \
	if test -h freex3d-$(PACKAGE_VERSION); then \
		rm -f freex3d-$(PACKAGE_VERSION); fi && \
	ln -s freex3d freex3d-$(PACKAGE_VERSION) && \
	tar cfz freex3d-minimal-$(PACKAGE_VERSION).tar.gz \
		freex3d-$(PACKAGE_VERSION)/* --exclude '.svn'

all: vtempl
