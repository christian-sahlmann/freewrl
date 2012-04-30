/*
=INSERT_TEMPLATE_HERE=

$Id: RenderTextures.c,v 1.45 2012/04/30 19:04:23 crc_canada Exp $

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

#define SET_TEXTURE_UNIT_AND_BIND(aaa,bbb) { \
     /* printf ("textureUnit %d texture %d\n",aaa,bbb);  */ \
    glActiveTexture(GL_TEXTURE0+aaa); \
    glBindTexture(GL_TEXTURE_2D,bbb); }



///* variables for keeping track of status */
//static int currentTextureUnit = 99;
//struct multiTexParams *textureParameterStack[MAX_MULTITEXTURE];
//void *textureParameterStack[MAX_MULTITEXTURE];
//typedef struct pRenderTextures{
///* variables for keeping track of status */
//int currentTextureUnit;// = 99;
//
//}* ppRenderTextures;

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
		p->currentTextureUnit = 99;
	}
}


/* function params */
static void haveTexCoord(struct X3D_TextureCoordinate *myTCnode);
static void passedInGenTex(struct textureVertexInfo *genTex);
static void haveMultiTexCoord(struct X3D_MultiTextureCoordinate *myMTCnode);
static void haveTexCoordGenerator (struct X3D_TextureCoordinate *myTCnode);

/* TextureGenerator node? if so, do it */
static void setupTexGen (struct X3D_TextureCoordinateGenerator *this) {
#if defined(IPHONE) || defined(_ANDROID ) || defined(GLES2)
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

#ifdef OLDCODE
OLDCODEstatic int setActiveTexture (int c, GLfloat thisTransparency) 
OLDCODE{
OLDCODE        struct multiTexParams *paramPtr;
OLDCODE	float allones[] = {1.0f, 1.0f, 1.0f, 1.0f};
OLDCODE	ppRenderTextures p;
OLDCODE	ttglobal tg = gglobal();
OLDCODE	p = (ppRenderTextures)tg->RenderTextures.prv;
OLDCODE
OLDCODE	if (tg->display.rdr_caps.av_multitexture) {
OLDCODE	    
OLDCODE	    if (c != p->currentTextureUnit) {
OLDCODE		  SET_TEXTURE_UNIT(c);
OLDCODE			if(0){
OLDCODE				GLint loc;
OLDCODE				GLint x;
OLDCODE				GLint useTex;
OLDCODE				s_shader_capabilities_t *csp;
OLDCODE				csp = getAppearanceProperties()->currentShaderProperties; 
OLDCODE				switch(c){
OLDCODE					case 0:
OLDCODE							loc = csp->Texture0; break;
OLDCODE					case 1:
OLDCODE							loc = csp->Texture1; break;
OLDCODE					case 2:
OLDCODE							loc = csp->Texture2; break;
OLDCODE					case 3:
OLDCODE							loc = csp->Texture3; break;
OLDCODE					default:
OLDCODE							loc = csp->Texture0; break;
OLDCODE				}
OLDCODE				useTex = csp->useTex;
OLDCODE				glActiveTexture(GL_TEXTURE0+c);
OLDCODE			     //glUniform1i(loc+c, c); 
OLDCODE				glUniform1i(loc,c);
OLDCODE			 if( useTex > -1){ 
OLDCODE				float fw_useTex[4]; 
OLDCODE				int jj; 
OLDCODE				for(jj=0;jj<4;jj++) fw_useTex[jj] = jj <= c ? 1.0f : 0.0f; 
OLDCODE				glUniform4f(useTex,fw_useTex[0],fw_useTex[1],fw_useTex[2],fw_useTex[3]); 
OLDCODE			  } 
OLDCODE			}
OLDCODE		  p->currentTextureUnit = c;
OLDCODE	    }
OLDCODE
OLDCODE	} else {
OLDCODE
OLDCODE	    // this should be already set
OLDCODE	    p->currentTextureUnit = 0;
OLDCODE	
OLDCODE	    #if defined(IPHONE) || defined(GLES2)
OLDCODE		/* printf ("forcing texture unit to zero and sending in uniform for shader \n"); */
OLDCODE		SET_TEXTURE_UNIT(0);
OLDCODE	    #endif
OLDCODE	}
OLDCODE
OLDCODE	/* ENABLE_TEXTURES */
OLDCODE	#ifndef SHADERS_2011
OLDCODE	FW_GL_ENABLE(GL_TEXTURE_2D);
OLDCODE	#endif
OLDCODE
OLDCODE
OLDCODE	/* is this a MultiTexture, or just a "normal" single texture?  When we
OLDCODE	   bind_image, we store a pointer for the texture parameters. It is
OLDCODE	   NULL, possibly different for MultiTextures */
OLDCODE
OLDCODE	if (tg->RenderTextures.textureParameterStack[c] == NULL) {
OLDCODE		#ifdef TEXVERBOSE
OLDCODE		printf ("setActiveTexture - simple texture NOT a MultiTexture \n"); 
OLDCODE		#endif
OLDCODE
OLDCODE                /* should we set the coloUr to 1,1,1,1 so that the material does not show
OLDCODE                   through a RGB texture?? */
OLDCODE                /* only do for the first texture if MultiTexturing */
OLDCODE                if (c == 0) {
OLDCODE			#ifdef TEXVERBOSE
OLDCODE			printf ("setActiveTexture - firsttexture  \n"); 
OLDCODE			#endif
OLDCODE
OLDCODE			/* flit in the Material transparency here */
OLDCODE			allones[3] = thisTransparency;
OLDCODE			do_glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (GLfloat *)allones);
OLDCODE                }
OLDCODE
OLDCODE
OLDCODE#ifndef SHADERS_2011
OLDCODE		FW_GL_TEXENVI (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
OLDCODE#endif /* SHADERS_2011 */
OLDCODE
OLDCODE	} else {
OLDCODE		paramPtr = (struct multiTexParams *)tg->RenderTextures.textureParameterStack[c];
OLDCODE
OLDCODE		/* is this texture unit active? ie is mode something other than "OFF"? */
OLDCODE		if (paramPtr->texture_env_mode != 0) {
OLDCODE
OLDCODE#if defined(IPHONE) || defined (_ANDROID) || defined(GLES2)
OLDCODE//printf ("skipping the texenvi - assume replace color and add in shader\n");
OLDCODE#else
OLDCODE		switch (paramPtr->texture_env_mode) {
OLDCODE			case GL_MODULATE:
OLDCODE				FW_GL_TEXENVI (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
OLDCODE				break;
OLDCODE			case GL_REPLACE:
OLDCODE				FW_GL_TEXENVI (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
OLDCODE				break;
OLDCODE
OLDCODE			default:	
OLDCODE			
OLDCODE			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
OLDCODE			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_COMBINE_RGB, paramPtr->combine_rgb);
OLDCODE
OLDCODE			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_SOURCE0_RGB, paramPtr->source0_rgb);
OLDCODE			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_OPERAND0_RGB, paramPtr->operand0_rgb);
OLDCODE
OLDCODE			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_SOURCE1_RGB, paramPtr->source1_rgb);
OLDCODE			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_OPERAND1_RGB, paramPtr->operand1_rgb);
OLDCODE
OLDCODE			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, paramPtr->combine_alpha);
OLDCODE			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, paramPtr->source0_alpha);
OLDCODE			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, paramPtr->operand0_alpha);
OLDCODE
OLDCODE			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_RGB_SCALE, paramPtr->rgb_scale);
OLDCODE			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_ALPHA_SCALE, paramPtr->alpha_scale);
OLDCODE
OLDCODE			/* do we need these for this mode? */
OLDCODE			if (paramPtr->source1_alpha != 0) 
OLDCODE				FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, paramPtr->source1_alpha);
OLDCODE			if (paramPtr->operand1_alpha != 0) 
OLDCODE				FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, paramPtr->operand1_alpha);
OLDCODE
OLDCODE			}
OLDCODE#endif
OLDCODE
OLDCODE
OLDCODE		} else {
OLDCODE			FW_GL_DISABLE(GL_TEXTURE_2D); /* DISABLE_TEXTURES */
OLDCODE			return FALSE;
OLDCODE		}
OLDCODE	}
OLDCODE
OLDCODE
OLDCODE	PRINT_GL_ERROR_IF_ANY("");
OLDCODE
OLDCODE	return TRUE;
OLDCODE}
#endif //OLDCODE

static int setActiveTexture (int c, GLfloat thisTransparency,  GLint *texUnit, GLint *texMode) 
{
	ttglobal tg = gglobal();

    /* which texture unit are we working on? */
    
    /* tie each fw_TextureX uniform into the correct texture unit */
    
    // here we assign the texture unit to a specific number. NOTE: in the current code, this will ALWAYS
    // be [0] = 0, [1] = 1; etc.
    texUnit[c] = c;
    
    
#ifdef TEXVERBOSE
    printf ("SET_TEXTURE_UNIT %d, boundTextureStack is %d, sending to uniform %d\n",c,boundTextureStack[c],
        getAppearanceProperties()->currentShaderProperties->Texture_unit_array[c]);
#endif
    
	/* is this a MultiTexture, or just a "normal" single texture?  When we
	   bind_image, we store a pointer for the texture parameters. It is
	   NULL, possibly different for MultiTextures */

	if (tg->RenderTextures.textureParameterStack[c].multitex_mode == INT_ID_UNDEFINED) {
        
    #ifdef TEXVERBOSE
		printf ("setActiveTexture - simple texture NOT a MultiTexture \n"); 
		#endif

                /* should we set the coloUr to 1,1,1,1 so that the material does not show
                   through a RGB texture?? */
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
        //printf ("muititex source for %d is %d\n",c,tg->RenderTextures.textureParameterStack[c].multitex_source);
		if (tg->RenderTextures.textureParameterStack[c].multitex_source != MTMODE_OFF) {
		} else {
			glDisable(GL_TEXTURE_2D); /* DISABLE_TEXTURES */
			return FALSE;
		}
	}


	PRINT_GL_ERROR_IF_ANY("");

	return TRUE;
}

void textureDraw_start(struct X3D_Node *texC, struct textureVertexInfo* genTex) {
	struct X3D_TextureCoordinate *myTCnode = NULL;

	#ifdef TEXVERBOSE
	printf ("textureDraw_start, textureStackTop %d texture[0] %d\n",gglobal()->RenderFuncs.textureStackTop,boundTextureStack[0]);
	printf ("	texC %p, genTex %p\n",texC,genTex);
	#endif

	/* is this generated textures, like an extrusion or IFS without a texCoord param? */
	if (texC == NULL) {
		passedInGenTex(genTex);

	/* hmmm - maybe this texCoord node exists? */
	} else {
		switch (texC->_nodeType) {
		case NODE_IndexedFaceSet: myTCnode = (struct X3D_TextureCoordinate *) X3D_INDEXEDFACESET(texC)->texCoord; break;
		case NODE_ElevationGrid: myTCnode = (struct X3D_TextureCoordinate *) X3D_ELEVATIONGRID(texC)->texCoord; break;
		case NODE_GeoElevationGrid: myTCnode = (struct X3D_TextureCoordinate *) X3D_GEOELEVATIONGRID(texC)->texCoord; break;
		case NODE_TriangleSet: myTCnode = (struct X3D_TextureCoordinate *) X3D_TRIANGLESET(texC)->texCoord; break;
		case NODE_TriangleFanSet: myTCnode = (struct X3D_TextureCoordinate *) X3D_TRIANGLEFANSET(texC)->texCoord; break;
		case NODE_TriangleStripSet: myTCnode = (struct X3D_TextureCoordinate *) X3D_TRIANGLESTRIPSET(texC)->texCoord; break;
		case NODE_IndexedTriangleSet: myTCnode = (struct X3D_TextureCoordinate *) X3D_INDEXEDTRIANGLESET(texC)->texCoord; break;
		case NODE_IndexedTriangleStripSet: myTCnode = (struct X3D_TextureCoordinate *) X3D_INDEXEDTRIANGLESTRIPSET(texC)->texCoord; break;
		case NODE_IndexedTriangleFanSet: myTCnode = (struct X3D_TextureCoordinate *) X3D_INDEXEDTRIANGLEFANSET(texC)->texCoord; break;
		default :printf ("textureDrawStart, unhandled node type...\n");
		

		}

		if (myTCnode == NULL) return;

		#ifdef TEXVERBOSE
		printf ("ok, texC->_nodeType is %s\n",stringNodeType(texC->_nodeType));
		printf ("myTCnode is of type %s\n",stringNodeType(myTCnode->_nodeType));
		#endif

		if (myTCnode->_nodeType == NODE_TextureCoordinate) {
			haveTexCoord(myTCnode);


		} else if (myTCnode->_nodeType == NODE_MultiTextureCoordinate) {
			haveMultiTexCoord((struct X3D_MultiTextureCoordinate *)myTCnode);

		} else {
			/* this has to be a TexureCoordinateGenerator node */
			haveTexCoordGenerator (myTCnode);
		}
	}
	PRINT_GL_ERROR_IF_ANY("");
	return;
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

		if (c != p->currentTextureUnit) {
			SET_TEXTURE_UNIT(c);
			//glActiveTexture(GL_TEXTURE0+c);
			//if(1){
			//	GLint useTex;
			//	s_shader_capabilities_t *csp;
			//	csp = getAppearanceProperties()->currentShaderProperties; 
			//	useTex = csp->useTex;
			// if( useTex > -1){ 
			//	float fw_useTex[4]; 
			//	int jj; 
			//	for(jj=0;jj<4;jj++) fw_useTex[jj] = 0.0f; //set textures off
			//	glUniform4f(useTex,fw_useTex[0],fw_useTex[1],fw_useTex[2],fw_useTex[3]); 
			//  } 
			//}

			p->currentTextureUnit = c;
		}

	        if (getThis_textureTransform()) end_textureTransform();
		FW_GL_DISABLECLIENTSTATE(GL_TEXTURE_COORD_ARRAY);
		#ifndef SHADERS_2011
		FW_GL_DISABLE(GL_TEXTURE_GEN_S);
		FW_GL_DISABLE (GL_TEXTURE_GEN_T);
		FW_GL_DISABLE(GL_TEXTURE_2D);
		#endif /* SHADERS_2011 */
	    }

	} else {

	        if (getThis_textureTransform()) end_textureTransform();
		FW_GL_DISABLECLIENTSTATE(GL_TEXTURE_COORD_ARRAY);

		/* not in OpenGL-ES 2.0... */
		#ifndef SHADERS_2011
			FW_GL_DISABLE(GL_TEXTURE_GEN_S);
			FW_GL_DISABLE (GL_TEXTURE_GEN_T);
			FW_GL_DISABLE(GL_TEXTURE_2D);
		#endif /* SHADERS_2011 */

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
	printf ("passedInGenTex, using passed in genTex, textureStackTop %d\n",textureStackTop);
	#endif 
 
	if (genTex->VA_arrays != NULL) {
       // printf ("passedInGenTex, A\n");
		for (c=0; c<=tg->RenderFuncs.textureStackTop; c++) {
            //printf ("passedInGenTex, c= %d\n",c);
			/* are we ok with this texture yet? */
			if (tg->RenderFuncs.boundTextureStack[c]!=0) {
                //printf ("passedInGenTex, B\n");
				if (setActiveTexture(c,getAppearanceProperties()->transparency,texUnit,texMode)) {
                    //printf ("passedInGenTex, C\n");
                    if (getThis_textureTransform()) start_textureTransform(getThis_textureTransform(),c);
                    SET_TEXTURE_UNIT_AND_BIND(c,tg->RenderFuncs.boundTextureStack[c]);
                   
                    FW_GL_TEXCOORD_POINTER (2,GL_FLOAT,0,genTex->VA_arrays);
                    sendClientStateToGPU(TRUE,GL_TEXTURE_COORD_ARRAY);
				}
			}
		}
	} else {
        //printf ("passedInGenTex, B\n");
		for (c=0; c<=tg->RenderFuncs.textureStackTop; c++) {
            //printf ("passedInGenTex, c=%d\n",c);
			/* are we ok with this texture yet? */
			if (tg->RenderFuncs.boundTextureStack[c]!=0) {
                //printf ("passedInGenTex, C, boundTextureStack %d\n",tg->RenderFuncs.boundTextureStack[c]);
				if (setActiveTexture(c,getAppearanceProperties()->transparency,texUnit,texMode)) {
                    //printf ("passedInGenTex, going to bind to texture %d\n",tg->RenderFuncs.boundTextureStack[c]);
        				if (getThis_textureTransform()) start_textureTransform(getThis_textureTransform(),c);
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
    for (i=0; i<tg->RenderFuncs.textureStackTop; i++) {
        //printf (" sending in i%d tu %d mode %d\n",i,i,tg->RenderTextures.textureParameterStack[i].multitex_mode);
        glUniform1i(getAppearanceProperties()->currentShaderProperties->TextureUnit[i],i);
        glUniform1i(getAppearanceProperties()->currentShaderProperties->TextureMode[i],tg->RenderTextures.textureParameterStack[i].multitex_mode);
    }
    
    
	PRINT_GL_ERROR_IF_ANY("");
}

static void haveTexCoord(struct X3D_TextureCoordinate *myTCnode) {
	int c;
    int i;
    GLint texUnit[MAX_MULTITEXTURE];
    GLint texMode[MAX_MULTITEXTURE];
	ttglobal tg = gglobal();
    
	#ifdef TEXVERBOSE
	printf ("have a NODE_TextureCoordinate\n");
	printf ("and this texture has %d points we have texture stacking depth of %d\n",myTCnode->point.n,tg->RenderFuncs.textureStackTop);
	#endif

	/* render the TextureCoordinate node for every texture in this node */

	COMPILE_TCNODE;

	for (c=0; c<=tg->RenderFuncs.textureStackTop; c++) {
		/* printf ("haveTexCoord, rendering node... \n"); */
		render_node ((void *)myTCnode);
		/* are we ok with this texture yet? */
		//printf ("haveTexCoord, boundTextureStack[c] = %d VBO %d\n",tg->RenderFuncs.boundTextureStack[c], myTCnode->__VBO);
		if (tg->RenderFuncs.boundTextureStack[c] !=0) {

			if (setActiveTexture(c,getAppearanceProperties()->transparency,texUnit,texMode)) {
	       			if (getThis_textureTransform()) start_textureTransform(getThis_textureTransform(),c);
                    if (myTCnode->__VBO != 0) {
                        struct textureVertexInfo mtf = {NULL,2,GL_FLOAT,0, NULL};
                        FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,myTCnode->__VBO);
                        passedInGenTex(&mtf);
                    } else {
                        SET_TEXTURE_UNIT_AND_BIND(c,tg->RenderFuncs.boundTextureStack[c]);
  
                        FW_GL_TEXCOORD_POINTER (2,GL_FLOAT,0,(float *)myTCnode->__compiledpoint.p);
                        sendClientStateToGPU (TRUE,GL_TEXTURE_COORD_ARRAY);
                    }
			}
		}
	}    
    for (i=0; i<tg->RenderFuncs.textureStackTop; i++) {
        glUniform1i(getAppearanceProperties()->currentShaderProperties->TextureUnit[i],i);
        glUniform1i(getAppearanceProperties()->currentShaderProperties->TextureMode[i],tg->RenderTextures.textureParameterStack[i].multitex_mode);
    }

	PRINT_GL_ERROR_IF_ANY("");
}
#ifdef OLDCODE
OLDCODEstatic void passedInGenTex(struct textureVertexInfo *genTex) {
OLDCODE	int c;
OLDCODE	ttglobal tg = gglobal();
OLDCODE	#ifdef TEXVERBOSE
OLDCODE	printf ("passedInGenTex, using passed in genTex\n");
OLDCODE	#endif 
OLDCODE 
OLDCODE	if (genTex->VA_arrays != NULL) {
OLDCODE		for (c=0; c<tg->RenderFuncs.textureStackTop; c++) {
OLDCODE			/* are we ok with this texture yet? */
OLDCODE			if (tg->RenderFuncs.boundTextureStack[c]!=0) {
OLDCODE				if (setActiveTexture(c,getAppearanceProperties()->transparency)) {
OLDCODE        				if (getThis_textureTransform()) start_textureTransform(getThis_textureTransform(),c);
OLDCODE					FW_GL_BINDTEXTURE(GL_TEXTURE_2D,tg->RenderFuncs.boundTextureStack[c]);
OLDCODE					FW_GL_TEXCOORD_POINTER (2,GL_FLOAT,0,genTex->VA_arrays);
OLDCODE					FW_GL_ENABLECLIENTSTATE (GL_TEXTURE_COORD_ARRAY);
OLDCODE				}
OLDCODE			}
OLDCODE		}
OLDCODE	} else {
OLDCODE
OLDCODE		for (c=0; c<tg->RenderFuncs.textureStackTop; c++) {
OLDCODE			/* are we ok with this texture yet? */
OLDCODE			if (tg->RenderFuncs.boundTextureStack[c]!=0) {
OLDCODE				if (setActiveTexture(c,getAppearanceProperties()->transparency)) {
OLDCODE        				if (getThis_textureTransform()) start_textureTransform(getThis_textureTransform(),c);
OLDCODE					FW_GL_BINDTEXTURE(GL_TEXTURE_2D,tg->RenderFuncs.boundTextureStack[c]);
OLDCODE					FW_GL_TEXCOORD_POINTER (genTex->TC_size, 
OLDCODE						genTex->TC_type,
OLDCODE						genTex->TC_stride,
OLDCODE						genTex->TC_pointer);
OLDCODE					FW_GL_ENABLECLIENTSTATE (GL_TEXTURE_COORD_ARRAY);
OLDCODE				}
OLDCODE			}
OLDCODE		}
OLDCODE	}
OLDCODE	PRINT_GL_ERROR_IF_ANY("");
OLDCODE	return;
OLDCODE}
OLDCODE
OLDCODE
OLDCODEstatic void haveTexCoord(struct X3D_TextureCoordinate *myTCnode) {
OLDCODE	int c;
OLDCODE	ttglobal tg = gglobal();
OLDCODE	#ifdef TEXVERBOSE
OLDCODE	printf ("have a NODE_TextureCoordinate\n");
OLDCODE	printf ("and this texture has %d points we have texture stacking depth of %d\n",myTCnode->point.n,tg->RenderFuncs.textureStackTop);
OLDCODE	#endif
OLDCODE
OLDCODE	/* render the TextureCoordinate node for every texture in this node */
OLDCODE	for (c=0; c<tg->RenderFuncs.textureStackTop; c++) {
OLDCODE		/* printf ("haveTexCoord, rendering node... \n"); */
OLDCODE		render_node ((void *)myTCnode);
OLDCODE		/* are we ok with this texture yet? */
OLDCODE		/* printf ("haveTexCoord, boundTextureStack[c] = %d\n",boundTextureStack[c]); */
OLDCODE		if (tg->RenderFuncs.boundTextureStack[c] !=0) {
OLDCODE
OLDCODE			if (setActiveTexture(c,getAppearanceProperties()->transparency)) {
OLDCODE	       			if (getThis_textureTransform()) start_textureTransform(getThis_textureTransform(),c);
OLDCODE				if (myTCnode->__VBO != 0) {
OLDCODE                                	struct textureVertexInfo mtf = {NULL,2,GL_FLOAT,0, NULL};
OLDCODE                                	FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,myTCnode->__VBO);
OLDCODE					passedInGenTex(&mtf);
OLDCODE				} else {
OLDCODE					FW_GL_BINDTEXTURE(GL_TEXTURE_2D,tg->RenderFuncs.boundTextureStack[c]);
OLDCODE					FW_GL_TEXCOORD_POINTER (2,GL_FLOAT,0,(float *)myTCnode->__compiledpoint.p);
OLDCODE					FW_GL_ENABLECLIENTSTATE (GL_TEXTURE_COORD_ARRAY);
OLDCODE				}
OLDCODE			}
OLDCODE		}
OLDCODE	}
OLDCODE	PRINT_GL_ERROR_IF_ANY("");
OLDCODE}
#endif //OLDCODE

static void haveMultiTexCoord(struct X3D_MultiTextureCoordinate *myMTCnode) {
	int c;
    GLint texUnit[MAX_MULTITEXTURE];
    GLint texMode[MAX_MULTITEXTURE];
	struct X3D_TextureCoordinate *myTCnode;
	ttglobal tg = gglobal();
	myTCnode = (struct X3D_TextureCoordinate *) myMTCnode; /* for now, in case of errors, this is set to an invalid value */

	#ifdef TEXVERBOSE
	printf ("MultiTextureCoordinate node, have %d texCoords\n",myMTCnode->texCoord.n);
	#endif
	
	/* render the TextureCoordinate node for every texture in this node */
	for (c=0; c<tg->RenderFuncs.textureStackTop; c++) {
		/* do we have enough textures in this MultiTextureCoordinate node? */
		if (c<myMTCnode->texCoord.n) {
			myTCnode = 
				(struct X3D_TextureCoordinate *) myMTCnode->texCoord.p[c];

			/* is this a valid textureCoord node, and not another
			   MultiTextureCoordinate node? */
			if ((myTCnode->_nodeType == NODE_TextureCoordinate) ||
			    (myTCnode->_nodeType == NODE_TextureCoordinateGenerator)) {
				render_node (X3D_NODE(myTCnode));
				/* are we ok with this texture yet? */
				if (tg->RenderFuncs.boundTextureStack[c] != 0) {
					if (setActiveTexture(c,getAppearanceProperties()->transparency,texUnit,texMode)) {
        					if (getThis_textureTransform()) start_textureTransform(getThis_textureTransform(),c);
						FW_GL_BINDTEXTURE(GL_TEXTURE_2D,tg->RenderFuncs.boundTextureStack[c]);
						FW_GL_TEXCOORD_POINTER (2,GL_FLOAT,0,(float *)myTCnode->__compiledpoint.p);
						FW_GL_ENABLECLIENTSTATE (GL_TEXTURE_COORD_ARRAY);
					}
				}
			#ifdef TEXVERBOSE
			} else {
				printf ("MultiTextureCoord, problem with %d as a child \n",myTCnode->_nodeType);
			#endif
			}
		#ifdef TEXVERBOSE
		} else {
			printf ("MultiTextureCoord, not enough children for the number of textures...\n");
		#endif
		}
		/* are we ok with this texture yet? */
		if (tg->RenderFuncs.boundTextureStack[c] != 0) {
			if (setActiveTexture(c,getAppearanceProperties()->transparency,texUnit,texMode)) {
        			if (getThis_textureTransform()) start_textureTransform(getThis_textureTransform(),c);

				FW_GL_BINDTEXTURE(GL_TEXTURE_2D,gglobal()->RenderFuncs.boundTextureStack[c]);
			
				/* do the texture coordinate stuff */
				if (myTCnode->_nodeType == NODE_TextureCoordinate) {
					FW_GL_TEXCOORD_POINTER (2,GL_FLOAT,0,(float *)myTCnode->__compiledpoint.p);
				} else if (myTCnode->_nodeType == NODE_TextureCoordinateGenerator) {
					setupTexGen ((struct X3D_TextureCoordinateGenerator*) myTCnode);
				}
			}
		}
	}
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
	       			if (getThis_textureTransform()) start_textureTransform(getThis_textureTransform(),c);
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

