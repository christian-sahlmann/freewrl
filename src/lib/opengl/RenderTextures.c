/*
=INSERT_TEMPLATE_HERE=

$Id: RenderTextures.c,v 1.32 2011/01/14 17:30:35 crc_canada Exp $

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
#include "../scenegraph/Component_Shape.h"
#include "../opengl/OpenGL_Utils.h"
#include "../scenegraph/RenderFuncs.h"

#include "OpenGL_Utils.h"
#include "Textures.h"
#include "Material.h"


/* variables for keeping track of status */
static int currentTextureUnit = 99;
struct multiTexParams *textureParameterStack[MAX_MULTITEXTURE];


/* function params */
static void haveTexCoord(struct X3D_TextureCoordinate *myTCnode);
static void passedInGenTex(struct textureVertexInfo *genTex);
static void haveMultiTexCoord(struct X3D_MultiTextureCoordinate *myMTCnode);
static void haveTexCoordGenerator (struct X3D_TextureCoordinate *myTCnode);

/* TextureGenerator node? if so, do it */
static void setupTexGen (struct X3D_TextureCoordinateGenerator *this) {
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
}

/* which texture unit are we going to use? is this texture not OFF?? Should we set the
   background coloUr??? Larry the Cucumber, help! */

static int setActiveTexture (int c, GLfloat thisTransparency) 
{
        struct multiTexParams *paramPtr;
	float allones[] = {1.0f, 1.0f, 1.0f, 1.0f};

	if (rdr_caps.av_multitexture) {
	    
	    if (c != currentTextureUnit) {
		SET_TEXTURE_UNIT(c);
		currentTextureUnit = c;
	    }

	} else {

	    // this should be already set
	    currentTextureUnit = 0;

	}

	/* ENABLE_TEXTURES */
	FW_GL_ENABLE(GL_TEXTURE_2D);

	/* is this a MultiTexture, or just a "normal" single texture?  When we
	   bind_image, we store a pointer for the texture parameters. It is
	   NULL, possibly different for MultiTextures */

	if (textureParameterStack[c] == NULL) {
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

			/* flit in the Material transparency here */
			allones[3] = thisTransparency;
			do_glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (GLfloat *)allones);
                }

		FW_GL_TEXENVI (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	} else {
		paramPtr = textureParameterStack[c];

		/* is this texture unit active? ie is mode something other than "OFF"? */
		if (paramPtr->texture_env_mode != 0) {

		switch (paramPtr->texture_env_mode) {
			case GL_MODULATE:
				FW_GL_TEXENVI (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				break;
			case GL_REPLACE:
				FW_GL_TEXENVI (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				break;

			default:	
			
			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_COMBINE_RGB, paramPtr->combine_rgb);

			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_SOURCE0_RGB, paramPtr->source0_rgb);
			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_OPERAND0_RGB, paramPtr->operand0_rgb);

			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_SOURCE1_RGB, paramPtr->source1_rgb);
			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_OPERAND1_RGB, paramPtr->operand1_rgb);

			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, paramPtr->combine_alpha);
			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, paramPtr->source0_alpha);
			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, paramPtr->operand0_alpha);

			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_RGB_SCALE, paramPtr->rgb_scale);
			FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_ALPHA_SCALE, paramPtr->alpha_scale);

			/* do we need these for this mode? */
			if (paramPtr->source1_alpha != 0) 
				FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, paramPtr->source1_alpha);
			if (paramPtr->operand1_alpha != 0) 
				FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, paramPtr->operand1_alpha);

			}

		} else {
			FW_GL_DISABLE(GL_TEXTURE_2D); /* DISABLE_TEXTURES */
			return FALSE;
		}
	}

	return TRUE;
}

void textureDraw_start(struct X3D_Node *texC, struct textureVertexInfo* genTex) {
	struct X3D_TextureCoordinate *myTCnode = NULL;

	#ifdef TEXVERBOSE
	printf ("textureDraw_start, textureStackTop %d texture[0] %d\n",textureStackTop,boundTextureStack[0]);
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
		printf ("ok, texC->_nodeType is %d\n",texC->_nodeType);
		printf ("myTCnode is of type %d\n",myTCnode->_nodeType);
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
}

/* lets disable textures here */
void textureDraw_end(void) {
	int c;

#ifdef TEXVERBOSE
	printf ("start of textureDraw_end\n");
#endif

	if (rdr_caps.av_multitexture) { // test the availability at runtime of multi textures

	    for (c=0; c<textureStackTop; c++) {

		if (c != currentTextureUnit) {
			SET_TEXTURE_UNIT(c);
			currentTextureUnit = c;
		}

	        if (this_textureTransform) end_textureTransform();
		FW_GL_DISABLECLIENTSTATE(GL_TEXTURE_COORD_ARRAY);
		FW_GL_DISABLE(GL_TEXTURE_GEN_S);
		FW_GL_DISABLE (GL_TEXTURE_GEN_T);
		FW_GL_DISABLE(GL_TEXTURE_2D);
	    }

	} else {

	        if (this_textureTransform) end_textureTransform();
		FW_GL_DISABLECLIENTSTATE(GL_TEXTURE_COORD_ARRAY);
		FW_GL_DISABLE(GL_TEXTURE_GEN_S);
		FW_GL_DISABLE (GL_TEXTURE_GEN_T);
		FW_GL_DISABLE(GL_TEXTURE_2D);

	}

	/* DISABLE_TEXTURES */
	/* setting this ENSURES that items, like the HUD, that are not within the normal
	   rendering path do not try and use textures... */
	textureStackTop = 0;

        FW_GL_MATRIX_MODE(GL_MODELVIEW);
}

/***********************************************************************************/


static void passedInGenTex(struct textureVertexInfo *genTex) {
	int c;

	#ifdef TEXVERBOSE
	printf ("passedInGenTex, using passed in genTex\n");
	#endif 
 
	if (genTex->VA_arrays != NULL) {
		for (c=0; c<textureStackTop; c++) {
			/* are we ok with this texture yet? */
			if (boundTextureStack[c]!=0) {
				if (setActiveTexture(c,appearanceProperties.transparency)) {
        				if (this_textureTransform) start_textureTransform(this_textureTransform,c);
					FW_GL_BINDTEXTURE(GL_TEXTURE_2D,boundTextureStack[c]);
					FW_GL_TEXCOORD_POINTER (2,GL_FLOAT,0,genTex->VA_arrays);
					FW_GL_ENABLECLIENTSTATE (GL_TEXTURE_COORD_ARRAY);
				}
			}
		}
	} else {

		for (c=0; c<textureStackTop; c++) {
			/* are we ok with this texture yet? */
			if (boundTextureStack[c]!=0) {
				if (setActiveTexture(c,appearanceProperties.transparency)) {
        				if (this_textureTransform) start_textureTransform(this_textureTransform,c);
					FW_GL_BINDTEXTURE(GL_TEXTURE_2D,boundTextureStack[c]);
					FW_GL_TEXCOORD_POINTER (genTex->TC_size, 
						genTex->TC_type,
						genTex->TC_stride,
						genTex->TC_pointer);
					FW_GL_ENABLECLIENTSTATE (GL_TEXTURE_COORD_ARRAY);
				}
			}
		}
	}
}

static void haveTexCoord(struct X3D_TextureCoordinate *myTCnode) {
	int c;

	#ifdef TEXVERBOSE
	printf ("have a NODE_TextureCoordinate\n");
	printf ("and this texture has %d points we have texture stacking depth of %d\n",myTCnode->point.n,textureStackTop);
	#endif

	/* render the TextureCoordinate node for every texture in this node */
	for (c=0; c<textureStackTop; c++) {
		/* printf ("haveTexCoord, rendering node... \n"); */
		render_node ((void *)myTCnode);
		/* are we ok with this texture yet? */
		/* printf ("haveTexCoord, boundTextureStack[c] = %d\n",boundTextureStack[c]); */
		if (boundTextureStack[c] !=0) {

			if (setActiveTexture(c,appearanceProperties.transparency)) {
	       			if (this_textureTransform) start_textureTransform(this_textureTransform,c);

				if (myTCnode->__VBO != 0) {
                                	struct textureVertexInfo mtf = {NULL,2,GL_FLOAT,0, NULL};
                                	FW_GL_BINDBUFFER(GL_ARRAY_BUFFER_ARB,myTCnode->__VBO);
					passedInGenTex(&mtf);
				} else {
					FW_GL_BINDTEXTURE(GL_TEXTURE_2D,boundTextureStack[c]);
					FW_GL_TEXCOORD_POINTER (2,GL_FLOAT,0,(float *)myTCnode->__compiledpoint.p);
					FW_GL_ENABLECLIENTSTATE (GL_TEXTURE_COORD_ARRAY);
				}
			}
		}
	}
}

static void haveMultiTexCoord(struct X3D_MultiTextureCoordinate *myMTCnode) {
	int c;
	struct X3D_TextureCoordinate *myTCnode;

	myTCnode = (struct X3D_TextureCoordinate *) myMTCnode; /* for now, in case of errors, this is set to an invalid value */

	#ifdef TEXVERBOSE
	printf ("MultiTextureCoordinate node, have %d texCoords\n",myMTCnode->texCoord.n);
	#endif
	
	/* render the TextureCoordinate node for every texture in this node */
	for (c=0; c<textureStackTop; c++) {
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
				if (boundTextureStack[c] != 0) {
					if (setActiveTexture(c,appearanceProperties.transparency)) {
        					if (this_textureTransform) start_textureTransform(this_textureTransform,c);
						FW_GL_BINDTEXTURE(GL_TEXTURE_2D,boundTextureStack[c]);
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
		if (boundTextureStack[c] != 0) {
			if (setActiveTexture(c,appearanceProperties.transparency)) {
        			if (this_textureTransform) start_textureTransform(this_textureTransform,c);

				FW_GL_BINDTEXTURE(GL_TEXTURE_2D,boundTextureStack[c]);
			
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

	#ifdef TEXVERBOSE
	printf ("have a NODE_TextureCoordinateGenerator\n");
	#endif
		
	/* render the TextureCoordinate node for every texture in this node */
	for (c=0; c<textureStackTop; c++) {
		render_node ((void *)myTCnode);
		/* are we ok with this texture yet? */
		if (boundTextureStack[c] != 0) {
			if (setActiveTexture(c,appearanceProperties.transparency)) {
	       			if (this_textureTransform) start_textureTransform(this_textureTransform,c);
				FW_GL_BINDTEXTURE(GL_TEXTURE_2D,boundTextureStack[c]);

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

