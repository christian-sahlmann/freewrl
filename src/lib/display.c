/*
  $Id: display.c,v 1.28 2009/12/28 00:55:41 couannette Exp $

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
#include <libFreeWRL.h>

#include "opengl/Textures.h"
#include "opengl/RasterFont.h"


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


#if defined (TARGET_AQUA)
/* display part specific to Mac */

CGLContextObj myglobalContext;
AGLContext aqglobalContext;

GLboolean cErr;

GDHandle gGDevice;

int ccurse = ACURSE;
int ocurse = ACURSE;

/* for handling Safari window changes at the top of the display event loop */
int PaneClipnpx;
int PaneClipnpy;
WindowPtr PaneClipfwWindow;
int PaneClipct;
int PaneClipcb;
int PaneClipcr;
int PaneClipcl;
int PaneClipwidth;
int PaneClipheight;
int PaneClipChanged = FALSE;
#endif

static char blankTexture[] = {0x40, 0x40, 0x40, 0xFF};


/**
 *  display_initialize: takes care of all the initialization process, 
 *                      creates the display thread and wait for it to complete
 *                      the OpenGL initialization and the Window creation.
 */
int display_initialize()
{
	memset(&rdr_caps, 0, sizeof(rdr_caps));

	/* FreeWRL parameters */
	fullscreen = fw_params.fullscreen;
	win_width = fw_params.width;
	win_height = fw_params.height;

#if !defined (TARGET_AQUA)
	/* make the window, get the OpenGL context */
#ifndef _MSC_VER
	if (!open_display()) {
		return FALSE;
	}

	if (!create_GLcontext()) {
		return FALSE;
	}

#endif

	if (!create_main_window(0 /*argc*/, NULL /*argv*/)) {
		return FALSE;
	}
#ifndef _MSC_VER
	bind_GLcontext();
#endif
#else
#ifdef OLDCODE
	if (RUNNINGASPLUGIN) {  // commented out
         	aglSetCurrentContext(aqglobalContext); 
	}
#endif
#endif /* TARGET_AQUA */

	if (!initialize_GL()) {
		return FALSE;
	}

#ifdef OLDCODE
	/* initialize raster font 
	   font is just an example picked up with xfontsel...
	   we handle only XFont here... we'll come with a cross
	   platform soon :)...
	 */
	if (!rf_xfont_init("-*-bitstream vera sans mono-medium-r-*-*-*-120-*-*-*-*-*-*")) {
		ERROR_MSG("could not initialize raster font\n");
	}
#endif

	/* create an empty texture, defaultBlankTexture, to be used when a texture is loading, or if it fails */
	glGenTextures(1,&defaultBlankTexture);
	glBindTexture (GL_TEXTURE_2D, defaultBlankTexture);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blankTexture);

	/* Display full initialized :P cool ! */
	display_initialized = TRUE;

	DEBUG_MSG("FreeWRL: running as a plugin: %s\n", BOOL_STR(isBrowserPlugin));

#if defined(FREEWRL_PLUGIN) && (defined(TARGET_X11) || defined(TARGET_MOTIF))
	if (RUNNINGASPLUGIN) {
		sendXwinToPlugin();
	}
#endif

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
 *   resize_GL: when the window is resized we have to update the GL viewport.
 */
GLvoid resize_GL(GLsizei width, GLsizei height)
{ 
    glViewport( 0, 0, width, height ); 
}

/**
 * On all platforms, when we don't have GLEW, we simulate it.
 * In any case we setup the rdr_capabilities struct.
 */
bool initialize_rdr_caps()
{
	/* OpenGL is initialized, context is created,
	   get some info, for later use ...*/
        rdr_caps.renderer   = (char *) glGetString(GL_RENDERER);
        rdr_caps.version    = (char *) glGetString(GL_VERSION);
        rdr_caps.vendor     = (char *) glGetString(GL_VENDOR);
	rdr_caps.extensions = (char *) glGetString(GL_EXTENSIONS);

#ifdef HAVE_LIBGLEW

	/* Initialize GLEW */
	{
	GLenum err;
	err = glewInit();
	if (GLEW_OK != err) {
		/* Problem: glewInit failed, something is seriously wrong. */
		ERROR_MSG("GLEW initialization error: %s\n", glewGetErrorString(err));
		return FALSE;
	}
	TRACE_MSG("GLEW initialization: version %s\n", glewGetString(GLEW_VERSION));

	rdr_caps.av_glsl_shaders = GLEW_ARB_fragment_shader;
	rdr_caps.av_multitexture = GLEW_ARB_multitexture;
	rdr_caps.av_occlusion_q = GLEW_ARB_occlusion_query;
	rdr_caps.av_npot_texture = GLEW_ARB_texture_non_power_of_two;
	rdr_caps.av_texture_rect = GLEW_ARB_texture_rectangle;
	}

#else
	/* Initialize renderer capabilities without GLEW */

	/* Shaders */
        rdr_caps.av_glsl_shaders = (strstr (rdr_caps.extensions, "GL_ARB_fragment_shader")!=0);
	
	/* Multitexturing */
	rdr_caps.av_multitexture = (strstr (rdr_caps.extensions, "GL_ARB_multitexture")!=0);

	/* Occlusion Queries */
	rdr_caps.av_occlusion_q = (strstr (rdr_caps.extensions, "GL_ARB_occlusion_query") !=0);

	/* Non-power-of-two textures */
	rdr_caps.av_npot_texture = (strstr (rdr_caps.extensions, "GL_ARB_texture_non_power_of_two") !=0);

	/* Texture rectangle (x != y) */
	rdr_caps.av_texture_rect = (strstr (rdr_caps.extensions, "GL_ARB_texture_rectangle") !=0);
#endif

	/* Max texture size */
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &rdr_caps.max_texture_size);
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &rdr_caps.texture_units);

	/* TODO: Update the code to use
	   - rdr_caps.max_texture_size
	   - rdr_caps.texture_units
	   instead of
	   - opengl_has_textureSize
	   - opengl_has_numTextureUnits
	*/

	/* User settings in environment */

	if (global_texture_size > 0) {
		DEBUG_MSG("Environment set texture size: %d", global_texture_size);
		rdr_caps.max_texture_size = global_texture_size;
	}

	/* Special drivers settings */

	if (strncmp(rdr_caps.renderer, "Intel GMA 9", 11) == 0) {
		if (rdr_caps.max_texture_size > 1024) rdr_caps.max_texture_size = 1024;
	}

	if (strncmp(rdr_caps.renderer, "NVIDIA GeForce2", 15) == 0) {
		if (rdr_caps.max_texture_size > 1024) rdr_caps.max_texture_size = 1024; 
	}

	/* print some debug infos */
	rdr_caps_dump(&rdr_caps);

	return TRUE;
}

void initialize_rdr_functions()
{
	/**
	 * WARNING:
	 *
	 * Linux OpenGL driver (Mesa or ATI or NVIDIA) and Windows driver
	 * will not initialize function pointers. So we use GLEW or we
	 * initialize them ourself.
	 *
	 * OSX 10.4 : same as above.
	 * OSX 10.5 and OSX 10.6 : Apple driver will initialize functions.
	 */

	
}

void rdr_caps_dump()
{
#ifdef VERBOSE
	{
		char *p, *pp;
		p = pp = STRDUP(rdr_caps.extensions);
		while (*pp != '\0') {
			if (*pp == ' ') *pp = '\n';
			pp++;
		}
		DEBUG_MSG ("OpenGL extensions : %s\n", p);
		FREE(p);
	}
#endif //VERBOSE

	DEBUG_MSG ("Shader support:       %s\n", BOOL_STR(rdr_caps.av_glsl_shaders));
	DEBUG_MSG ("Multitexture support: %s\n", BOOL_STR(rdr_caps.av_multitexture));
	DEBUG_MSG ("Occlusion support:    %s\n", BOOL_STR(rdr_caps.av_occlusion_q));
	DEBUG_MSG ("Max texture size      %d\n", rdr_caps.max_texture_size);
	DEBUG_MSG ("Texture units         %d\n", rdr_caps.texture_units);
}
