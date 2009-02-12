# src dir Makefile.am
#
# $Id: Makefile.am,v 1.5 2009/02/12 18:33:32 istakenv Exp $
#
# Conditional build of sub-directories

DIST_SUBDIRS = lib bin plugin sound message libeai
SUBDIRS = lib bin 

SUBDIRS+=$(MAYBE_PLUGIN)
SUBDIRS+=$(MAYBE_LIBEAI)

if OS_LINUX
SUBDIRS+=sound message
endif
