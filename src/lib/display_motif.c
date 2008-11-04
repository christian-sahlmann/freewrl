/*******************************************************************
 *
 * FreeX3D support library
 *
 * display_motif.c
 *
 * $Id: display_motif.c,v 1.3 2008/11/04 00:40:34 couannette Exp $
 *
 *******************************************************************/

#include "config.h"
#include "system.h"
#include "display.h"
#include "internal.h"

/* display part specific to X11/Motif */

XtAppContext Xtcx;

int open_display()
{
    return FALSE;
}

int create_main_window()
{
    return FALSE;
}

int create_GL_context()
{
    return FALSE;
}
