# src dir Makefile.am
#
# $Id: Makefile.am,v 1.4 2008/12/31 13:08:15 couannette Exp $
#
# Conditional build of sub-directories

DIST_SUBDIRS = lib bin plugin sound message
SUBDIRS = lib bin 

SUBDIRS+=$(MAYBE_PLUGIN)

if OS_LINUX
SUBDIRS+=sound message
endif
