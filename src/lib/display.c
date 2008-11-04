/*******************************************************************
 *
 * FreeX3D support library
 *
 * display.c
 *
 * $Id: display.c,v 1.3 2008/11/04 00:40:34 couannette Exp $
 *
 *******************************************************************/

#include "config.h"
#include "system.h"
#include "display.h"
#include "internal.h"

/* common function between display_x11, display_motif and display_aqua */

int win_height = 0; /* window */
int win_width = 0;
int fullscreen = FALSE;
int view_height = 0; /* viewport */
int view_width = 0;

char *window_title = NULL;

int mouse_x;
int mouse_y;

int show_mouse;

int display_initialize()
{
    // Default width / height
    if (!win_width)
	win_width = 800;
    if (!win_height)
	win_height = 600;

    if (!open_display())
	return FALSE;

    if (!create_GL_context())
	return FALSE;

    if (!create_main_window())
	return FALSE;

    return TRUE;
}
