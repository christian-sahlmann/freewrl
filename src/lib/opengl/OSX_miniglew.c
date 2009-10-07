/*******************************************************************
 *
 * FreeWRL Apple specific support library
 *
 * main.c
 *
 * $Id: OSX_miniglew.c,v 1.2 2009/10/07 19:51:23 crc_canada Exp $
 *
 *******************************************************************/


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

#ifdef TARGET_AQUA

# include <OpenGL/OpenGL.h>
# include <OpenGL/CGLTypes.h>
# include <AGL/AGL.h>
# include "OSX_miniglew.h"
# include "../ui/aquaInt.h"

static char * glExtensions;

int GLEW_ARB_multitexture;
int GLEW_ARB_occlusion_query;
int GLEW_ARB_fragment_shader;
extern int opengl_has_textureSize;
extern int opengl_has_numTextureUnits;
extern int displayOpenGLErrors;



GLenum glewInit(void) {
	GLint checktexsize;

	/* get extensions for runtime */
	/* printf ("OpenGL - getting extensions\n"); */
	
        glExtensions = (char *)glGetString(GL_EXTENSIONS);

	/* Shaders */
        GLEW_ARB_fragment_shader = (strstr (glExtensions, "GL_ARB_fragment_shader")!=0);
 
	/* Multitexturing */
        GLEW_ARB_multitexture = (strstr (glExtensions, "GL_ARB_multitexture")!=0);

	/* Occlusion Queries */
	GLEW_ARB_occlusion_query = (strstr (glExtensions, "GL_ARB_occlusion_query") !=0);

	
	/* first, lets check to see if we have a max texture size yet */

	/* note - we reduce the max texture size on computers with the (incredibly inept) Intel GMA 9xx chipsets - like the Intel
	   Mac minis, and macbooks up to November 2007 */
	if (opengl_has_textureSize<=0) { 
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &checktexsize); 
		if (getenv("FREEWRL_256x256_TEXTURES")!= NULL) checktexsize = 256; 
		if (getenv("FREEWRL_512x512_TEXTURES")!= NULL) checktexsize = 512; 
		opengl_has_textureSize = -opengl_has_textureSize; 
		if (opengl_has_textureSize == 0) opengl_has_textureSize = checktexsize; 
		if (opengl_has_textureSize > checktexsize) opengl_has_textureSize = checktexsize; 
		if (strncmp((const char *)glGetString(GL_RENDERER),"NVIDIA GeForce2",strlen("NVIDIA GeForce2")) == 0) { 
		/* 	printf ("possibly reducing texture size because of NVIDIA GeForce2 chip\n"); */ 
			if (opengl_has_textureSize > 1024) opengl_has_textureSize = 1024; 
		}  
		if (strncmp((const char *)glGetString(GL_RENDERER),"Intel GMA 9",strlen("Intel GMA 9")) == 0) { 
		/* 	printf ("possibly reducing texture size because of Intel GMA chip\n"); */
			if (opengl_has_textureSize > 1024) opengl_has_textureSize = 1024;
		}
		if (displayOpenGLErrors) printf ("CHECK_MAX_TEXTURE_SIZE, ren %s ver %s ven %s ts %d\n",glGetString(GL_RENDERER), glGetString(GL_VERSION), glGetString(GL_VENDOR),opengl_has_textureSize);
		setMenuButton_texSize (opengl_has_textureSize);
	} 


	/* how many Texture units? */
	if ((strstr (glExtensions, "GL_ARB_texture_env_combine")!=0) &&
		(strstr (glExtensions,"GL_ARB_multitexture")!=0)) {

		glGetIntegerv(GL_MAX_TEXTURE_UNITS,&opengl_has_numTextureUnits);

#define MAX_MULTITEXTURE 10 /* from headers.h */
		if (opengl_has_numTextureUnits > MAX_MULTITEXTURE) {
			printf ("init_multitexture_handling - reducing number of multitexs from %d to %d\n",
				opengl_has_numTextureUnits,MAX_MULTITEXTURE);
			opengl_has_numTextureUnits = MAX_MULTITEXTURE;
		}
		/* printf ("can do multitexture we have %d units\n",opengl_has_numTextureUnits); */


		/* we assume that GL_TEXTURE*_ARB are sequential. Lets to a little check */
		if ((GL_TEXTURE0 +1) != GL_TEXTURE1) {
			printf ("Warning, code expects GL_TEXTURE0 to be 1 less than GL_TEXTURE1\n");
		} 
	}

#define VERBOSE

	#ifdef VERBOSE
	{
		char *p = glExtensions;
		while (*p != '\0') {
			if (*p == ' ') *p = '\n';
			p++;
		}
	}
	printf ("extensions %s\n",glExtensions);
	printf ("Shader support:       "); if (GLEW_ARB_fragment_shader) printf ("TRUE\n"); else printf ("FALSE\n");
	printf ("Multitexture support: "); if (GLEW_ARB_multitexture) printf ("TRUE\n"); else printf ("FALSE\n");
	printf ("Occlusion support:    "); if (GLEW_ARB_occlusion_query) printf ("TRUE\n"); else printf ("FALSE\n");
	printf ("max texture size      %d\n",opengl_has_textureSize);
	printf ("texture units         %d\n",opengl_has_numTextureUnits);
	#endif	

	return GLEW_OK;
}

char *glewGetErrorString(GLenum err){

	return "OpenGL Error";
}
#endif

