/*******************************************************************
 *
 * FreeX3D support library
 *
 * main.c
 *
 * $Id: main.c,v 1.4 2008/11/03 14:14:12 couannette Exp $
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
int be_collision = FALSE;
char *keypress_string = NULL;

/**
 * Variables that should go into their respective component file
 */
int EAIverbose = FALSE;


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
void initFreewrl()
{
}

/**
 * Explicit exit routine
 */
void closeFreewrl()
{
}

/**
 *
 */
void setFullPath(const char* file)
{
}

/**
 *
 */
void setEaiVerbose() {
	EAIverbose = FALSE;
}

void setLineWidth(float lwidth) {
        gl_linewidth = lwidth;
}
