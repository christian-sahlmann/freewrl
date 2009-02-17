# src dir Makefile.am
#
# $Id: Makefile.am,v 1.6 2009/02/17 16:08:19 istakenv Exp $
#
# Conditional build of sub-directories

DIST_SUBDIRS = lib bin plugin sound message libeai
SUBDIRS = lib bin 

SUBDIRS+=$(MAYBE_PLUGIN)
SUBDIRS+=$(MAYBE_LIBEAI)

if OS_LINUX
SUBDIRS+=$(MAYBE_SOUND)
SUBDIRS+=message
endif
