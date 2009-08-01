/*
=INSERT_TEMPLATE_HERE=

$Id: display_win32.c,v 1.2 2009/08/01 09:45:39 couannette Exp $

FreeWRL support library.
Display (X11) initialization.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>


/*********************  Header Files  *********************/

#include "display_win32.h"
/* OpenGL globals, defines, and prototypes */ 

/**
 * Create OpenGL context
 */
int create_GL_context()
{


    return TRUE;
}

int initialize_gl_context()
{
    if (!ghWnd) {
	ERROR_MSG("window not initialized, can't initialize OpenGL context.\n");
	return FALSE;
    }
    return TRUE;
}
/* int display_initialize(); /* mb */
int open_display()
{
	return TRUE;
} /* mb */

/*int create_main_window(); /* mb */
/*int create_GL_context(); /* mb */
/*int initialize_gl_context(); /* mb */
/*int initialize_viewport(); /* mb */

/*void resetGeometry(); /*  */

/*void setScreenDim(int wi, int he); /* */

