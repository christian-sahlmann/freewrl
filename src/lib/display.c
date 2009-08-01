/*
=INSERT_TEMPLATE_HERE=

$Id: display.c,v 1.11 2009/08/01 09:45:39 couannette Exp $

FreeWRL support library.
Display (X11/Motif or OSX/Aqua) initialization.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>


/* common function between display_x11, display_motif and display_aqua */

int win_height = 0; /* window */
int win_width = 0;
int fullscreen = FALSE;
int view_height = 0; /* viewport */
int view_width = 0;

int screenWidth = 0; /* screen */
int screenHeight = 0;

double screenRatio = 1.5;

char *window_title = NULL;

int mouse_x;
int mouse_y;

int show_mouse;

int xPos = 0;
int yPos = 0;


int display_initialize()
{
    // Default width / height
    if (!win_width)
	win_width = 800;
    if (!win_height)
	win_height = 600;

    #ifndef TARGET_AQUA
    if (!open_display())
	return FALSE;
    #endif

    #ifndef TARGET_AQUA
    if (!create_GL_context())
	return FALSE;
    #else
    printf ("SKIPPING CREATE_GL_CONTEXT\n");
    #endif

    if (!create_main_window())
	return FALSE;

    #ifndef TARGET_AQUA
    if (!initialize_gl_context()) {
	return FALSE;
    }
    #endif

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

#if (defined TARGET_WIN32)
    return create_main_window_win32();
#endif

}

int initialize_viewport()
{
    glViewport(0, 0, win_width, win_height);
    return TRUE;
}

void setGeometry_from_cmdline(const char *gstring)
{
    int c;
    c = sscanf(gstring,"%dx%d+%d+%d", &win_width, &win_height, &xPos, &yPos);
    /* tell OpenGL what the screen dims are */
    setScreenDim(win_width,win_height);
}
FILE *theFile = NULL;

/* set internal variables for screen sizes, and calculate frustum */
void setScreenDim(int wi, int he)
{
    screenWidth = wi;
    screenHeight = he;

    if (screenHeight != 0) screenRatio = (double) screenWidth/(double) screenHeight;
    else screenRatio =  screenWidth;
}
