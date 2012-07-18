/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Texturing.c,v 1.27 2012/07/18 01:15:17 crc_canada Exp $

X3D Texturing Component

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

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../opengl/Textures.h"
#include "../opengl/OpenGL_Utils.h"
#include "../scenegraph/Component_Shape.h"
#include "../scenegraph/RenderFuncs.h"


/* verify the TextureCoordinateGenerator node - if the params are ok, then the internal
   __compiledmode is NOT zero. If there are problems, the __compiledmode IS zero */

void render_TextureCoordinateGenerator(struct X3D_TextureCoordinateGenerator *node) {
	char *modeptr;

	if NODE_NEEDS_COMPILING {
		MARK_NODE_COMPILED

		modeptr = node->mode->strptr;

		/* make the __compiledmode reflect actual OpenGL parameters */
		if(strcmp("SPHERE-REFLECT-LOCAL",modeptr)==0) {
			node->__compiledmode = GL_SPHERE_MAP;
		} else if(strcmp("SPHERE-REFLECT",modeptr)==0) {
			node->__compiledmode = GL_SPHERE_MAP;
		} else if(strcmp("SPHERE-LOCAL",modeptr)==0) {
			node->__compiledmode = GL_SPHERE_MAP;
		} else if(strcmp("SPHERE",modeptr)==0) {
			node->__compiledmode = GL_SPHERE_MAP;
		} else if(strcmp("CAMERASPACENORMAL",modeptr)==0) {
			node->__compiledmode = GL_NORMAL_MAP;
		} else if(strcmp("CAMERASPACEPOSITION",modeptr)==0) {
			node->__compiledmode = GL_OBJECT_LINEAR;
		} else if(strcmp("CAMERASPACEREFLECTION",modeptr)==0) {
			node->__compiledmode = GL_REFLECTION_MAP;
		} else if(strcmp("COORD-EYE",modeptr)==0) {
			node->__compiledmode = GL_EYE_LINEAR;
		} else if(strcmp("COORD",modeptr)==0) {
			node->__compiledmode = GL_EYE_LINEAR;
		} else if(strcmp("NOISE-EYE",modeptr)==0) {
			node->__compiledmode = GL_EYE_LINEAR;
		} else if(strcmp("NOISE",modeptr)==0) {
			node->__compiledmode = GL_EYE_LINEAR;
		} else {
			printf ("TextureCoordinateGenerator - error - %s invalid as a mode\n",modeptr);
		}
	}
}


void render_PixelTexture (struct X3D_PixelTexture *node) {
	loadTextureNode(X3D_NODE(node),NULL);
	gglobal()->RenderFuncs.textureStackTop=1; /* not multitexture - should have saved to boundTextureStack[0] */
}

void render_ImageTexture (struct X3D_ImageTexture *node) {
	/* printf ("render_ImageTexture, global Transparency %f\n",getAppearanceProperties()->transparency); */
	loadTextureNode(X3D_NODE(node),NULL);
	gglobal()->RenderFuncs.textureStackTop=1; /* not multitexture - should have saved to boundTextureStack[0] */
}

void render_MultiTexture (struct X3D_MultiTexture *node) {
	loadMultiTexture(node);
}

void render_MovieTexture (struct X3D_MovieTexture *node) {
#ifdef HAVE_TO_REIMPLEMENT_MOVIETEXTURES
	/* really simple, the texture number is calculated, then simply sent here.
	   The boundTextureStack field is sent, and, made current */

	/*  if this is attached to a Sound node, tell it...*/
	sound_from_audioclip = FALSE;

	loadTextureNode(X3D_NODE(node),NULL);
	gglobal()->RenderFuncs.boundTextureStack[gglobal()->RenderFuncs.textureStackTop] = node->__ctex;
	/* not multitexture, should have saved to boundTextureStack[0] */
#else /* HAVE_TO_REIMPLEMENT_MOVIETEXTURES */
	loadTextureNode(X3D_NODE(node),NULL);
#endif
	
	gglobal()->RenderFuncs.textureStackTop=1;
}
