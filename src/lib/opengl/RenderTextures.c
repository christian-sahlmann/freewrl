/*
=INSERT_TEMPLATE_HERE=

$Id: RenderTextures.c,v 1.60 2012/07/17 17:00:41 crc_canada Exp $

Texturing during Runtime 
texture enabling - works for single texture, for multitexture. 

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
#include "../opengl/OpenGL_Utils.h"
#include "../scenegraph/Component_Shape.h"
#include "../scenegraph/RenderFuncs.h"

#include "Textures.h"
#include "Material.h"

#ifdef TEXVERBOSE
#define SET_TEXTURE_UNIT_AND_BIND(aaa,bbb) { \
	printf ("textureUnit %d texture %d at %d\n",aaa,bbb,__LINE__); \
	glActiveTexture(GL_TEXTURE0+aaa); \
	glBindTexture(GL_TEXTURE_2D,bbb); }
#else
#define SET_TEXTURE_UNIT_AND_BIND(aaa,bbb) { \
	glActiveTexture(GL_TEXTURE0+aaa); \
	glBindTexture(GL_TEXTURE_2D,bbb); }
#endif


void *RenderTextures_constructor(){
	void *v = malloc(sizeof(struct pRenderTextures));
	memset(v,0,sizeof(struct pRenderTextures));
	return v;
}
void RenderTextures_init(struct tRenderTextures *t){
	//t->textureParameterStack[];
	t->prv = RenderTextures_constructor();
	{
		ppRenderTextures p = (ppRenderTextures)t->prv;
		/* variables for keeping track of status */
	}
}


/* function params */
static void haveTexCoord(struct X3D_TextureCoordinate *myTCnode);
static void passedInGenTex(struct textureVertexInfo *genTex);
static void haveMultiTexCoord(struct X3D_MultiTextureCoordinate *myMTCnode);
static void haveTexCoordGenerator (struct X3D_TextureCoordinate *myTCnode);

/* TextureGenerator node? if so, do it */
static void setupTexGen (struct X3D_TextureCoordinateGenerator *this) {
#if defined(GL_ES_VERSION_2_0)

printf ("skipping setupTexGen\n");
#else
	switch (this->__compiledmode) {
	case GL_OBJECT_LINEAR:
	case GL_EYE_LINEAR:
	case GL_REFLECTION_MAP:
	case GL_SPHERE_MAP:
	case GL_NORMAL_MAP:
		FW_GL_TEXGENI(GL_S, GL_TEXTURE_GEN_MODE,this->__compiledmode);
		FW_GL_TEXGENI(GL_T,GL_TEXTURE_GEN_MODE,this->__compiledmode);                      
		FW_GL_ENABLE(GL_TEXTURE_GEN_S);
		FW_GL_ENABLE(GL_TEXTURE_GEN_T);
	break;
	default: {}
		/* printf ("problem with compiledmode %d\n",this->__compiledmode); */
	}
#endif
}

/* which texture unit are we going to use? is this texture not OFF?? Should we set the
   background coloUr??? Larry the Cucumber, help! */


static int setActiveTexture (int c, GLfloat thisTransparency,  GLint *texUnit, GLint *texMode) 
{
	ttglobal tg = gglobal();

	/* which texture unit are we working on? */
    
	/* tie each fw_TextureX uniform into the correct texture unit */
    
	/* here we assign the texture unit to a specific number. NOTE: in the current code, this will ALWAYS
	 * be [0] = 0, [1] = 1; etc. */
	texUnit[c] = c;

#ifdef TEXVERBOSE
	if (getAppearanceProperties()->currentShaderProperties != NULL) {
		printf ("setActiveTexture %d, boundTextureStack is %d, sending to uniform %d\n",c,
			tg->RenderFuncs.boundTextureStack[c],
			getAppearanceProperties()->currentShaderProperties->TextureUnit[c]);
	} else {
		printf ("setActiveTexture %d, boundTextureStack is %d, sending to uniform [NULL--No Shader]\n",c,
			tg->RenderFuncs.boundTextureStack[c]);
	}
#endif
    
	/* is this a MultiTexture, or just a "normal" single texture?  When we
	 * bind_image, we store a pointer for the texture parameters. It is
	 * NULL, possibly different for MultiTextures */

	if (tg->RenderTextures.textureParameterStack[c].multitex_mode == INT_ID_UNDEFINED) {
        
		#ifdef TEXVERBOSE
		printf ("setActiveTexture - simple texture NOT a MultiTexture \n"); 
		#endif

		/* should we set the coloUr to 1,1,1,1 so that the material does not show
		 * through a RGB texture?? */
		/* only do for the first texture if MultiTexturing */
		if (c == 0) {
			#ifdef TEXVERBOSE
			printf ("setActiveTexture - firsttexture  \n"); 
			#endif
			texMode[c]= GL_MODULATE;
		} else {
			texMode[c]=GL_ADD;
		}

	} else {
	/* printf ("muititex source for %d is %d\n",c,tg->RenderTextures.textureParameterStack[c].multitex_source); */
		if (tg->RenderTextures.textureParameterStack[c].multitex_source != MTMODE_OFF) {
		} else {
			glDisable(GL_TEXTURE_2D); /* DISABLE_TEXTURES */
			return FALSE;
		}
	}


	PRINT_GL_ERROR_IF_ANY("");

	return TRUE;
}


void textureDraw_start(struct textureVertexInfo* genTex) {
		passedInGenTex(genTex);
}


/* lets disable textures here */
void textureDraw_end(void) {
	int c;
	ppRenderTextures p;
	ttglobal tg = gglobal();
	p = (ppRenderTextures)tg->RenderTextures.prv;

#ifdef TEXVERBOSE
	printf ("start of textureDraw_end\n");
#endif

	if (tg->display.rdr_caps.av_multitexture) { // test the availability at runtime of multi textures

	    for (c=0; c<tg->RenderFuncs.textureStackTop; c++) {

		FW_GL_DISABLECLIENTSTATE(GL_TEXTURE_COORD_ARRAY);

	    }

	} else {

		FW_GL_DISABLECLIENTSTATE(GL_TEXTURE_COORD_ARRAY);

	}

	/* DISABLE_TEXTURES */
	/* setting this ENSURES that items, like the HUD, that are not within the normal
	   rendering path do not try and use textures... */
	tg->RenderFuncs.textureStackTop = 0;

        FW_GL_MATRIX_MODE(GL_MODELVIEW);
}

/***********************************************************************************/

static void passedInGenTex(struct textureVertexInfo *genTex) {
	int c;
	int i;
	GLint texUnit[MAX_MULTITEXTURE];
	GLint texMode[MAX_MULTITEXTURE];
	ttglobal tg = gglobal();

	#ifdef TEXVERBOSE
	printf ("passedInGenTex, using passed in genTex, textureStackTop %d\n",tg->RenderFuncs.textureStackTop);
	#endif 

    /* simple shapes, like Boxes and Cones and Spheres will have pre-canned arrays */
	if (genTex->pre_canned_textureCoords != NULL) {
       // printf ("passedInGenTex, A\n");
		for (c=0; c<tg->RenderFuncs.textureStackTop; c++) {
            //printf ("passedInGenTex, c= %d\n",c);
			/* are we ok with this texture yet? */
			if (tg->RenderFuncs.boundTextureStack[c]!=0) {
                //printf ("passedInGenTex, B\n");
				if (setActiveTexture(c,getAppearanceProperties()->transparency,texUnit,texMode)) {
                    struct X3D_Node *tt = getThis_textureTransform();
                    //printf ("passedInGenTex, C\n");
                    if (tt!=NULL) do_textureTransform(tt,c);
                    SET_TEXTURE_UNIT_AND_BIND(c,tg->RenderFuncs.boundTextureStack[c]);
                   
                    FW_GL_TEXCOORD_POINTER (2,GL_FLOAT,0,genTex->pre_canned_textureCoords);
                    sendClientStateToGPU(TRUE,GL_TEXTURE_COORD_ARRAY);
				}
			}
}
	} else {
        //printf ("passedInGenTex, B\n");
		for (c=0; c<tg->RenderFuncs.textureStackTop; c++) {
            //printf ("passedInGenTex, c=%d\n",c);
			/* are we ok with this texture yet? */
			if (tg->RenderFuncs.boundTextureStack[c]!=0) {
                //printf ("passedInGenTex, C, boundTextureStack %d\n",tg->RenderFuncs.boundTextureStack[c]);
				if (setActiveTexture(c,getAppearanceProperties()->transparency,texUnit,texMode)) {
                    //printf ("passedInGenTex, going to bind to texture %d\n",tg->RenderFuncs.boundTextureStack[c]);
                    struct X3D_Node *tt = getThis_textureTransform();
                    if (tt!=NULL) do_textureTransform(tt,c);
                    SET_TEXTURE_UNIT_AND_BIND(c,tg->RenderFuncs.boundTextureStack[c]);
                    
					FW_GL_TEXCOORD_POINTER (genTex->TC_size, 
						genTex->TC_type,
						genTex->TC_stride,
						genTex->TC_pointer);
					sendClientStateToGPU(TRUE,GL_TEXTURE_COORD_ARRAY);
				}
			}
		}

	}

	if (getAppearanceProperties()->currentShaderProperties != NULL) {
	    for (i=0; i<tg->RenderFuncs.textureStackTop; i++) {
        	//printf (" sending in i%d tu %d mode %d\n",i,i,tg->RenderTextures.textureParameterStack[i].multitex_mode);
		glUniform1i(getAppearanceProperties()->currentShaderProperties->TextureUnit[i],i);
		glUniform1i(getAppearanceProperties()->currentShaderProperties->TextureMode[i],tg->RenderTextures.textureParameterStack[i].multitex_mode);
	    }
	#ifdef TEXVERBOSE
	} else {
		printf (" NOT sending in %d i+tu+mode because currentShaderProperties is NULL\n",tg->RenderFuncs.textureStackTop);
	#endif
	}
    
    
	PRINT_GL_ERROR_IF_ANY("");
}


static void haveTexCoordGenerator (struct X3D_TextureCoordinate *myTCnode) {
	int c;

	GLint texUnit[MAX_MULTITEXTURE];
	GLint texMode[MAX_MULTITEXTURE];
	ttglobal tg = gglobal();
	#ifdef TEXVERBOSE
	printf ("have a NODE_TextureCoordinateGenerator\n");
	#endif
		
	/* render the TextureCoordinate node for every texture in this node */
	for (c=0; c<tg->RenderFuncs.textureStackTop; c++) {
		render_node ((void *)myTCnode);
		/* are we ok with this texture yet? */
		if (tg->RenderFuncs.boundTextureStack[c] != 0) {
			if (setActiveTexture(c,getAppearanceProperties()->transparency,texUnit,texMode)) {
	       			if (getThis_textureTransform()) do_textureTransform(getThis_textureTransform(),c);
				FW_GL_BINDTEXTURE(GL_TEXTURE_2D,tg->RenderFuncs.boundTextureStack[c]);

				setupTexGen((struct X3D_TextureCoordinateGenerator*) myTCnode);
			}
		}
	}
}

/*
  Some functions in Textures.c should be moved here
  and possibly renamed to textureDraw_###### :

  loadBackgroundTextures
  loadTextureBackgroundTextures

  Reason: it seems those function do a render job
  whereas other load functions do a fetch & load job.
*/

