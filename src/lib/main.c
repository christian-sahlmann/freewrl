/*******************************************************************
 *
 * FreeX3D support library
 *
 * main.c
 *
 * $Id: main.c,v 1.6 2008/11/19 18:19:12 couannette Exp $
 *
 *******************************************************************/

#include "config.h"
#include "system.h"
#include "display.h"
#include "internal.h"

#include "libFreeX3D.h"


/**
 * General variables
 */

/**
 * library initialization
 */
void __attribute__ ((constructor)) libFreeX3D_init(void)
{
}

/**
 * library exit routine
 */
void __attribute__ ((destructor)) libFreeX3D_fini(void)
{
}

/**
 * Explicit initialization
 */
int initFreeX3D()
{
    if (!display_initialize()) {
	ERROR("error in initialization.\n");
	return FALSE;
    }
    return TRUE;
}

/**
 * Explicit exit routine
 */
void closeFreeX3D()
{
}
