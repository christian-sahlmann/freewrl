/*******************************************************************
 *
 * FreeX3D support library
 *
 * display.c
 *
 * $Id: display.c,v 1.2 2008/11/03 14:14:12 couannette Exp $
 *
 *******************************************************************/

#include "config.h"
#include "system.h"
#include "display.h"
#include "internal.h"

/* common function between display_x11, display_motif and display_aqua */

int feHeight = 0; /* screen height and width */
int feWidth = 0;

int fullscreen = FALSE;	/* do fullscreen rendering? */

float gl_linewidth = 1.0;
