/*******************************************************************
 *
 * FreeX3D support library
 *
 * libFreeX3D.h
 *
 * Main include file that:
 *  - collect all external dependencies
 *  - collect all internal declarations
 *
 * This file cannot be installed/used by another program for now.
 * All config.h stuff should be removed before.
 *
 *******************************************************************/

#ifndef __LIBFREEX3D_MAIN_H__
#define __LIBFREEX3D_MAIN_H__


/* 
 * All system specfic stuff
 */
#include "libFreeX3D_system.h"

/*
 * All display (X11 or Mac) specific stuff
 */
#include "libFreeX3D_display.h"

/*
 * All FreeX3D declarations
 */
#include "libFreeX3D_decl.h"

/*
 * Version embedded
 */
extern const char *libFreeX3D_get_version();


#endif /* __LIBFREEX3D_MAIN_H__ */
