/*
=INSERT_TEMPLATE_HERE=

$Id: display.c,v 1.5 2008/12/05 13:20:52 couannette Exp $

FreeX3D support library.
Display (X11/Motif or OSX/Aqua) initialization.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>


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

    if (!initialize_gl_context()) {
	return FALSE;
    }

    if (!initialize_viewport()) {
	return FALSE;
    }

    return TRUE;
}

int create_main_window()
{
    /* theses flags are exclusive */

#if (defined TARGET_X11)
    return create_main_window_x11();
#endif

#if (defined TARGET_MOTIF)
    return create_main_window_motif();
#endif

#if (defined TARGET_AQUA)
    return create_main_window_aqua();
#endif
}

int initialize_viewport()
{
    glViewport(0, 0, win_width, win_height);
    return TRUE;
}
