/*
=INSERT_TEMPLATE_HERE=

$Id: display_win32.c,v 1.4 2009/10/05 15:07:23 crc_canada Exp $

FreeWRL support library.
Display (X11) initialization.

*/


/****************************************************************************
    This file is part of the FreeWRL/FreeX3D Distribution.

    Copyright 2009 CRC Canada. (http://www.crc.gc.ca)

    FreeWRL/FreeX3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FreeWRL/FreeX3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FreeWRL/FreeX3D.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/



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

