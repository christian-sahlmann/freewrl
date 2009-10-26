/*
  $Id: display.c,v 1.16 2009/10/26 17:48:43 couannette Exp $

  FreeWRL support library.
  Display (X11/Motif or OSX/Aqua) initialization.

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
#include <system_threads.h>
#include <display.h>
#include <internal.h>
#include <threads.h>


bool display_initialized = FALSE;

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

int shutterGlasses = 0; /* stereo shutter glasses */
int quadbuff_stereo_mode = 0;

s_renderer_capabilities_t rdr_caps;

float myFps = 0.0;
char myMenuStatus[MAXSTAT];

GLenum _global_gl_err;

/**
 *  display_initialize: takes care of all the initialization process, 
 *                      creates the display thread and wait for it to complete
 *                      the OpenGL initialization and the Window creation.
 */
int display_initialize()
{
	memset(&rdr_caps, 0, sizeof(rdr_caps));

	/* make the window, get the OpenGL context */
	if (!open_display()) {
		return FALSE;
	}

	if (!create_GLcontext()) {
		return FALSE;
	}


	if (!create_main_window(0 /*argc*/, NULL /*argv*/)) {
		return FALSE;
	}

	bind_GLcontext();

	if (!initialize_GL()) {
		return FALSE;
	}

	/* Display full initialized :P cool ! */
	display_initialized = TRUE;

	return TRUE;
}

/**
 *   setGeometry_from_cmdline: scan command line arguments (X11 convention), to
 *                             set up the window dimensions.
 */
void setGeometry_from_cmdline(const char *gstring)
{
    int c;
    c = sscanf(gstring,"%dx%d+%d+%d", &win_width, &win_height, &xPos, &yPos);
    /* tell OpenGL what the screen dims are */
    setScreenDim(win_width,win_height);
}

/**
 *   setScreenDim: set internal variables for screen sizes, and calculate frustum
 */
void setScreenDim(int wi, int he)
{
    screenWidth = wi;
    screenHeight = he;

    if (screenHeight != 0) screenRatio = (double) screenWidth/(double) screenHeight;
    else screenRatio =  screenWidth;
}

/**
 *   create_GLcontext: create the main OpenGL context.
 *                     TODO: finish implementation for Mac and Windows.
 */
bool create_GLcontext()
{	
	int direct_rendering = TRUE;

	fw_thread_dump();

#if defined(TARGET_X11) || defined(TARGET_MOTIF)

#ifdef DO_MULTI_OPENGL_THREADS
	direct_rendering = FALSE;
#endif

	GLcx = glXCreateContext(Xdpy, Xvi, NULL, direct_rendering);
	if (!GLcx) {
		ERROR_MSG("can't create OpenGL context.\n");
		return FALSE;
	}
	if (glXIsDirect(Xdpy, GLcx)) {
		TRACE_MSG("glX: direct rendering enabled\n");
	}
#endif
	
	return TRUE;
}

/**
 *   resize_GL: when the window is resized we have to update the GL viewport.
 */
GLvoid resize_GL(GLsizei width, GLsizei height)
{ 
    glViewport( 0, 0, width, height ); 
}

/**
 *   bind_GLcontext: attache the OpenGL context to the main window.
 *                   TODO: finish implementation for Mac and Windows.
 */
bool bind_GLcontext()
{
	fw_thread_dump();

#if defined(TARGET_X11) || defined(TARGET_MOTIF)
	if (!Xwin) {
		ERROR_MSG("window not initialized, can't initialize OpenGL context.\n");
		return FALSE;
	}
	if (!glXMakeCurrent(Xdpy, GLwin, GLcx)) {
		ERROR_MSG("bind_GLcontext: can't set OpenGL context for this thread %d (glXMakeCurrent: %s).\n", fw_thread_id(), GL_ERROR_MSG);
		return FALSE;
	}
#endif

#if defined(TARGET_AQUA)
	return aglSetCurrentContext(aqglobalContext);
#endif

#if defined(TARGET_WIN32)
	return wglMakeCurrent(ghDC, ghRC);
#endif

	return TRUE;
}
