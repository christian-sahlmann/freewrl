/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Texturing.c,v 1.13 2010/09/29 20:11:48 crc_canada Exp $

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
#include "../scenegraph/Component_Shape.h"


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

void render_TextureCoordinate(struct X3D_TextureCoordinate *node) {
	int i;
	int op;
	struct SFVec2f oFp;

	#ifdef TEXVERBOSE
	struct SFVec2f nFp;
	#endif

	float *fptr;


	#ifdef TEXVERBOSE
	printf ("rendering TextureCoordinate node __compiledpoint %d\n",node->__compiledpoint);
	printf ("tcin %d tcin_count %d oldpoint.n %d\n",global_tcin, global_tcin_count, node->point.n);
	#endif

	/* is node the statusbar? we should *always* have a global_tcin textureIndex */
	if (global_tcin == 0) return;

	/* note: IF this TextureCoordinate is USEd in more than one node, WE CAN NOT "pre-compile" the
	   coordinates, because, potentially the order of texture coordinate usage would be different. 
	   So, in the case where there is more than 1 parent, we have to re-calculate this ordering 
	   every time a texture is displayed */

	if (NODE_NEEDS_COMPILING || (node->__lastParent != global_tcin_lastParent)) {

		MARK_NODE_COMPILED

		if (node->__compiledpoint.n == 0) {
			node->__compiledpoint.n = global_tcin_count;
			FREE_IF_NZ(node->__compiledpoint.p);
		} 

		/* possibly, if we are using VBOs, we might have an issue with freed memory */
		/* so we do this in 2 steps */
		if (!node->__compiledpoint.p) {
			node->__compiledpoint.p = (struct SFVec2f *) MALLOC (sizeof(float) *2 * node->__compiledpoint.n);
		}
	
		fptr = (float *) node->__compiledpoint.p;
		
		/* ok, we have a bunch of triangles, loop through and stream the texture coords
		   into a format that matches 1 for 1, the coordinates */
	
		for (i=0; i<global_tcin_count; i++) {
			op = global_tcin[i];
	
			/* bounds check - is the tex coord greater than the number of points? 	*/
			/* node should have been checked before hand...				*/
			if (op >= node->point.n) {
				#ifdef TEXVERBOSE
				printf ("renderTextureCoord - op %d npoints %d\n",op,node->point.n);
				#endif
				*fptr = 0.0f; fptr++; 
				*fptr = 0.0f; fptr++; 
			} else {
				oFp = node->point.p[op];
	
				#ifdef TEXVERBOSE
				printf ("TextureCoordinate copying %d to %d\n",op,i);	
				printf ("	op %f %f\n",oFp.c[0], oFp.c[1]);
				#endif
	
				*fptr = oFp.c[0]; fptr++; *fptr = oFp.c[1]; fptr++;
			}
		}
		
			
		#ifdef TEXVERBOSE
		for (i=0; i<global_tcin_count; i++) {
			nFp = node->__compiledpoint.p[i];
			printf ("checking... %d %f %f\n",i,nFp.c[0], nFp.c[1]);
		}
		#endif

		/* We doing VBOs? */
		if (global_use_VBOs) {
			if (node->__VBO == 0) {
				GLuint tmp;
				/* do this in 2 steps to get around 32/64 bit OSX warnings */
				glGenBuffers(1,&tmp);
				node->__VBO = tmp;
			}

			glBindBufferARB(GL_ARRAY_BUFFER_ARB,node->__VBO);
			glBufferDataARB(GL_ARRAY_BUFFER_ARB,sizeof (float)*2*global_tcin_count, node->__compiledpoint.p, GL_STATIC_DRAW_ARB);
			FREE_IF_NZ(node->__compiledpoint.p);
		}
	}


	/* keep this last parent around, so that MAYBE we do not have to re-do the compiled node */
	node->__lastParent = global_tcin_lastParent;

	if (node->__compiledpoint.n < global_tcin_count) {
		printf ("TextureCoordinate - problem %d < %d\n",node->__compiledpoint.n,global_tcin_count);
	}

}

void render_PixelTexture (struct X3D_PixelTexture *node) {
	loadTextureNode(X3D_NODE(node),NULL);
	textureStackTop=1; /* not multitexture - should have saved to boundTextureStack[0] */
}

void render_ImageTexture (struct X3D_ImageTexture *node) {
	/* printf ("render_ImageTexture, global Transparency %f\n",appearanceProperties.transparency); */
	loadTextureNode(X3D_NODE(node),NULL);
	textureStackTop=1; /* not multitexture - should have saved to boundTextureStack[0] */
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
	boundTextureStack[textureStackTop] = node->__ctex;
	/* not multitexture, should have saved to boundTextureStack[0] */
#else /* HAVE_TO_REIMPLEMENT_MOVIETEXTURES */
	loadTextureNode(X3D_NODE(node),NULL);
#endif
	
	textureStackTop=1;
}
