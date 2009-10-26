/*
  $Id: OSX_miniglew.c,v 1.4 2009/10/26 17:48:43 couannette Exp $

  FreeWRL support library.
  OpenGL extensions detection and setup.

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

/**
 * On all platforms, when we don't have GLEW, we simulate it.
 * In any case we setup the rdr_capabilities struct.
 */

#include <config.h>
#include <system.h>
#include <system_threads.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#ifdef TARGET_AQUA
# include <OpenGL/OpenGL.h>
# include <OpenGL/CGLTypes.h>
# include <AGL/AGL.h>
# include "OSX_miniglew.h"
# include "../ui/aquaInt.h"
#endif //TARGET_AQUA

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
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		/* Problem: glewInit failed, something is seriously wrong. */
		ERROR_MSG("GLEW initialization error: %s\n", glewGetErrorString(err));
		return FALSE;
	}
	TRACE_MSG("GLEW initialization: version %s\n", glewGetString(GLEW_VERSION));

	rdr_caps.av_glsl_shader = GLEW_ARB_fragment_shader;
	rdr_caps.av_multitexture = GLEW_ARB_multitexture;
	rdr_caps.av_occlusion_q = GLEW_ARB_occlusion_query;
	rdr_caps.av_npot_texture = GLEW_ARB_texture_non_power_of_two;
	rdr_caps.av_texture_rect = GLEW_ARB_texture_rectangle;

#else
	/* Initialize renderer capabilities without GLEW */

	/* Shaders */
        rdr_caps.av_glsl_shader = (strstr (rdr_caps.extensions, "GL_ARB_fragment_shader")!=0);
	
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

	DEBUG_MSG ("Shader support:       %s\n", BOOL_STR(rdr_caps.av_glsl_shader));
	DEBUG_MSG ("Multitexture support: %s\n", BOOL_STR(rdr_caps.av_multitexture));
	DEBUG_MSG ("Occlusion support:    %s\n", BOOL_STR(rdr_caps.av_occlusion_q));
	DEBUG_MSG ("Max texture size      %d\n", opengl_has_textureSize);
	DEBUG_MSG ("Texture units         %d\n", opengl_has_numTextureUnits);

#endif //HAVE_LIBGLEW
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
