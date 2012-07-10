/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Texturing.c,v 1.26 2012/07/10 18:40:26 crc_canada Exp $

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

#ifdef OLDCODE
OLDCODEvoid render_TextureCoordinate(struct X3D_TextureCoordinate *node) {
OLDCODE	int i;
OLDCODE	int op;
OLDCODE	struct SFVec2f oFp;
OLDCODE
OLDCODE	#ifdef TEXVERBOSE
OLDCODE	struct SFVec2f nFp;
OLDCODE	#endif
OLDCODE
OLDCODE	float *fptr;
OLDCODE	ttglobal tg = gglobal();
OLDCODE
OLDCODE	#ifdef TEXVERBOSE
OLDCODE	printf ("rendering TextureCoordinate node __compiledpoint %p\n",node->__compiledpoint);
OLDCODE	printf ("tcin %d tcin_count %d oldpoint.n %d\n",tg->Textures.global_tcin, tg->Textures.global_tcin_count, node->point.n);
OLDCODE	#endif
OLDCODE
OLDCODE	/* is node the statusbar? we should *always* have a global_tcin textureIndex */
OLDCODE	if (tg->Textures.global_tcin == 0) return;
OLDCODE
OLDCODE	/* note: IF this TextureCoordinate is USEd in more than one node, WE CAN NOT "pre-compile" the
OLDCODE	   coordinates, because, potentially the order of texture coordinate usage would be different. 
OLDCODE	   So, in the case where there is more than 1 parent, we have to re-calculate this ordering 
OLDCODE	   every time a texture is displayed */
OLDCODE
OLDCODE	if (NODE_NEEDS_COMPILING || (node->__lastParent != tg->Textures.global_tcin_lastParent)) {
OLDCODE
OLDCODE		MARK_NODE_COMPILED
OLDCODE
OLDCODE		if (node->__compiledpoint.n == 0) {
OLDCODE			node->__compiledpoint.n = tg->Textures.global_tcin_count;
OLDCODE			FREE_IF_NZ(node->__compiledpoint.p);
OLDCODE		} 
OLDCODE
OLDCODE		/* possibly, if we are using VBOs, we might have an issue with freed memory */
OLDCODE		/* so we do this in 2 steps */
OLDCODE		if (!node->__compiledpoint.p) {
OLDCODE			node->__compiledpoint.p = (struct SFVec2f *) MALLOC (struct SFVec2f *, sizeof(struct SFVec2f) * node->__compiledpoint.n);
OLDCODE		}
OLDCODE	
OLDCODE		fptr = (float *) node->__compiledpoint.p;
OLDCODE		
OLDCODE		/* ok, we have a bunch of triangles, loop through and stream the texture coords
OLDCODE		   into a format that matches 1 for 1, the coordinates */
OLDCODE	
OLDCODE		for (i=0; i<tg->Textures.global_tcin_count; i++) {
OLDCODE			op = tg->Textures.global_tcin[i];
OLDCODE	
OLDCODE			/* bounds check - is the tex coord greater than the number of points? 	*/
OLDCODE			/* node should have been checked before hand...				*/
OLDCODE			if (op >= node->point.n) {
OLDCODE				#ifdef TEXVERBOSE
OLDCODE				printf ("renderTextureCoord - op %d npoints %d\n",op,node->point.n);
OLDCODE				#endif
OLDCODE				*fptr = 0.0f; fptr++; 
OLDCODE				*fptr = 0.0f; fptr++; 
OLDCODE			} else {
OLDCODE				oFp = node->point.p[op];
OLDCODE	
OLDCODE				#ifdef TEXVERBOSE
OLDCODE				printf ("TextureCoordinate copying %d to %d\n",op,i);	
OLDCODE				printf ("	op %f %f\n",oFp.c[0], oFp.c[1]);
OLDCODE				#endif
OLDCODE	
OLDCODE				*fptr = oFp.c[0]; fptr++; *fptr = oFp.c[1]; fptr++;
OLDCODE			}
OLDCODE		}
OLDCODE		
OLDCODE			
OLDCODE		#ifdef TEXVERBOSE
OLDCODE		for (i=0; i<tg->Textures.global_tcin_count; i++) {
OLDCODE			nFp = node->__compiledpoint.p[i];
OLDCODE			printf ("checking... %d %f %f\n",i,nFp.c[0], nFp.c[1]);
OLDCODE		}
OLDCODE		#endif
OLDCODE
OLDCODE		/* We doing VBOs? */
OLDCODE#ifdef SHADERS_2011
OLDCODE		 {
OLDCODE			if (node->__VBO == 0) {
OLDCODE				GLuint tmp;
OLDCODE				/* do this in 2 steps to get around 32/64 bit OSX warnings */
OLDCODE				glGenBuffers(1,&tmp);
OLDCODE				node->__VBO = tmp;
OLDCODE			}
OLDCODE
OLDCODE
OLDCODE/* 
OLDCODEdebugging code
OLDCODE
OLDCODEprintf ("textureCoordinate, filling in buffer...\n");
OLDCODEprintf ("global_tcin_count %d\n",tg->Textures.global_tcin_count);
OLDCODEprintf ("node cp.n %d\n",node->__compiledpoint.n);
OLDCODE{
OLDCODEint i;
OLDCODEfloat *tp = node->__compiledpoint.p;
OLDCODEprintf ("TC:\n");
OLDCODEfor (i=0; i< tg->Textures.global_tcin_count; i++) {
OLDCODEprintf ("       %d: %4.3f ",i,*tp); tp++; printf ("%4.3f\n",*tp); tp++;
OLDCODE
OLDCODE}
OLDCODE}
OLDCODE*/
OLDCODE
OLDCODE			FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,node->__VBO);
OLDCODE			glBufferData(GL_ARRAY_BUFFER,sizeof (float)*2*tg->Textures.global_tcin_count, node->__compiledpoint.p, GL_STATIC_DRAW);
OLDCODE			FREE_IF_NZ(node->__compiledpoint.p);
OLDCODE		}
OLDCODE#endif
OLDCODE	}
OLDCODE
OLDCODE
OLDCODE	/* keep this last parent around, so that MAYBE we do not have to re-do the compiled node */
OLDCODE	node->__lastParent = tg->Textures.global_tcin_lastParent;
OLDCODE
OLDCODE	if (node->__compiledpoint.n < tg->Textures.global_tcin_count) {
OLDCODE		printf ("TextureCoordinate - problem %d < %d\n",node->__compiledpoint.n,tg->Textures.global_tcin_count);
OLDCODE	}
OLDCODE}
#endif //OLDCODE

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
