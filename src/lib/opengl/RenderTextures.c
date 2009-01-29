/*
=INSERT_TEMPLATE_HERE=

$Id: RenderTextures.c,v 1.3 2009/01/29 21:14:40 crc_canada Exp $

Texturing during Runtime 
texture enabling - works for single texture, for multitexture. 

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"

#include "Textures.h"
/* #include "readpng.h" */

#undef TEXVERBOSE


/* variables for keeping track of status */
static int currentTextureUnit = 99;
static int textureEnabled = FALSE;
void *texParams[MAX_MULTITEXTURE];


/* function params */
void haveTexCoord(struct X3D_IndexedFaceSet *texC, struct X3D_TextureCoordinate *myTCnode);
void passedInGenTex(GLfloat *genTex);
void haveMultiTexCoord(struct X3D_IndexedFaceSet *texC);
void haveTexCoordGenerator (struct X3D_IndexedFaceSet *texC, struct X3D_TextureCoordinate *myTCnode);

/* TextureGenerator node? if so, do it */
void setupTexGen (struct X3D_TextureCoordinateGenerator *this) {
	switch (this->__compiledmode) {
	case GL_OBJECT_LINEAR:
	case GL_EYE_LINEAR:
	case GL_REFLECTION_MAP:
	case GL_SPHERE_MAP:
	case GL_NORMAL_MAP:
                                glTexGeni(GL_S, GL_TEXTURE_GEN_MODE,this->__compiledmode);
                                glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,this->__compiledmode);                      
                                glEnable(GL_TEXTURE_GEN_S);
                                glEnable(GL_TEXTURE_GEN_T);
	break;
	default: {}
		/* printf ("problem with compiledmode %d\n",this->__compiledmode); */
	}
}

/* which texture unit are we going to use? is this texture not OFF?? Should we set the
   background coloUr??? Larry the Cucumber, help! */

int setActiveTexture (int c, GLfloat thisTransparency) {
        struct multiTexParams *paramPtr;
	float allones[] = {1.0, 1.0, 1.0, 1.0};

	if (c != currentTextureUnit) {
		glActiveTexture(GL_TEXTURE0+c);
		glClientActiveTexture(GL_TEXTURE0+c); /* for TextureCoordinates */
		currentTextureUnit = c;
	}

	/* ENABLE_TEXTURES */
	glEnable(GL_TEXTURE_2D);

	/* is this a MultiTexture, or just a "normal" single texture?  When we
	   bind_image, we store a pointer for the texture parameters. It is
	   NULL, possibly different for MultiTextures */

	if (texParams[c] == NULL) {
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

		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	} else {
		paramPtr = (struct multiTexParams *) texParams[c];

		/* is this texture unit active? ie is mode something other than "OFF"? */
		if (paramPtr->texture_env_mode != 0) {

		switch (paramPtr->texture_env_mode) {
			case GL_MODULATE:
				glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				break;
			case GL_REPLACE:
				glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				break;

			default:	
			
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, paramPtr->combine_rgb);

			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, paramPtr->source0_rgb);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, paramPtr->operand0_rgb);

			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, paramPtr->source1_rgb);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, paramPtr->operand1_rgb);

			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, paramPtr->combine_alpha);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, paramPtr->source0_alpha);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, paramPtr->operand0_alpha);

			glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, paramPtr->rgb_scale);
			glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, paramPtr->alpha_scale);

			/* do we need these for this mode? */
			if (paramPtr->source1_alpha != 0) 
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, paramPtr->source1_alpha);
			if (paramPtr->operand1_alpha != 0) 
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, paramPtr->operand1_alpha);

			}

		} else {
			glDisable(GL_TEXTURE_2D); /* DISABLE_TEXTURES */
			return FALSE;
		}
	}

	return TRUE;
}

void textureDraw_start(struct X3D_IndexedFaceSet *texC, GLfloat *genTex) {
	struct X3D_TextureCoordinate *myTCnode;

	#ifdef TEXVERBOSE
	printf ("textureDraw_start, texture_count %d texture[0] %d\n",texture_count,bound_textures[0]);
	#endif


	/* is this generated textures, like an extrusion or IFS without a texCoord param? */
	if (texC == NULL) {
		passedInGenTex(genTex);

	/* hmmm - maybe this texCoord node exists? */
	} else {
		myTCnode = (struct X3D_TextureCoordinate *) texC->texCoord;

		#ifdef TEXVERBOSE
		printf ("ok, texC->_nodeType is %d\n",texC->_nodeType);
		printf ("myTCnode is of type %d\n",myTCnode->_nodeType);
		#endif

		if (myTCnode->_nodeType == NODE_TextureCoordinate) {
			haveTexCoord(texC,myTCnode);

		} else if (myTCnode->_nodeType == NODE_MultiTextureCoordinate) {
			haveMultiTexCoord(texC);

		} else {
			/* this has to be a TexureCoordinateGenerator node */
			haveTexCoordGenerator (texC, myTCnode);
		}
	}
}

/* lets disable textures here */
void textureDraw_end(void) {
	int c;

	#ifdef TEXVERBOSE
	printf ("start of textureDraw_end\n");
	#endif
	for (c=0; c<texture_count; c++) {
		if (c != currentTextureUnit) {
			glActiveTexture(GL_TEXTURE0+c);
			glClientActiveTexture(GL_TEXTURE0+c); /* for TextureCoordinates */
			currentTextureUnit = c;
		}

	        if (this_textureTransform) end_textureTransform(this_textureTransform,c);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                /*glTexGeni(GL_S, GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
                glTexGeni(GL_T, GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);*/
		glDisable(GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_2D);
	}
	/* DISABLE_TEXTURES */

        GL_MATRIX_MODE(GL_MODELVIEW);
}

/***********************************************************************************/


void passedInGenTex(GLfloat *genTex) {
	int c;

	#ifdef TEXVERBOSE
	printf ("textureDraw_start, using passed in genTex\n");
	#endif

	for (c=0; c<texture_count; c++) {
		/* are we ok with this texture yet? */
		if (bound_textures[c]!=0) {
			if (setActiveTexture(c,1.0)) {
        			if (this_textureTransform) start_textureTransform(this_textureTransform,c);
				glBindTexture(GL_TEXTURE_2D,bound_textures[c]);
				glTexCoordPointer (2,GL_FLOAT,0,genTex);
				glEnableClientState (GL_TEXTURE_COORD_ARRAY);
			}
		}
	}
}


void haveTexCoord(struct X3D_IndexedFaceSet *texC, struct X3D_TextureCoordinate *myTCnode) {
	int c;

	#ifdef TEXVERBOSE
	printf ("have a NODE_TextureCoordinate\n");
	printf ("and this texture has %d points we have texture stacking depth of %d\n",myTCnode->point.n,texture_count);
	#endif

	/* render the TextureCoordinate node for every texture in this node */
	for (c=0; c<texture_count; c++) {
		/* printf ("haveTexCoord, rendering node... \n"); */
		render_node (texC->texCoord);
		/* are we ok with this texture yet? */
		/* printf ("haveTexCoord, bound_textures[c] = %d\n",bound_textures[c]); */
		if (bound_textures[c] !=0) {
			if (setActiveTexture(c,1.0)) {
	       			if (this_textureTransform) start_textureTransform(this_textureTransform,c);
				glBindTexture(GL_TEXTURE_2D,bound_textures[c]);
				glTexCoordPointer (2,GL_FLOAT,0,myTCnode->__compiledpoint.p);
				glEnableClientState (GL_TEXTURE_COORD_ARRAY);
			}
		}
	}
}

void haveMultiTexCoord(struct X3D_IndexedFaceSet *texC) {
	int c;
	struct X3D_MultiTextureCoordinate *myMTCnode;
	struct X3D_TextureCoordinate *myTCnode;

	myMTCnode = (struct X3D_MultiTextureCoordinate *) texC->texCoord;
	myTCnode = (struct X3D_TextureCoordinate *) myMTCnode; /* for now, in case of errors, this is set to an invalid value */

	#ifdef TEXVERBOSE
	printf ("MultiTextureCoordinate node, have %d texCoords\n",myMTCnode->texCoord.n);
	#endif
	
	/* render the TextureCoordinate node for every texture in this node */
	for (c=0; c<texture_count; c++) {
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
				if (bound_textures[c] != 0) {
					if (setActiveTexture(c,1.0)) {
        					if (this_textureTransform) start_textureTransform(this_textureTransform,c);
						glBindTexture(GL_TEXTURE_2D,bound_textures[c]);
						glTexCoordPointer (2,GL_FLOAT,0,myTCnode->__compiledpoint.p);
						glEnableClientState (GL_TEXTURE_COORD_ARRAY);
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
		if (bound_textures[c] != 0) {
			if (setActiveTexture(c,1.0)) {
        			if (this_textureTransform) start_textureTransform(this_textureTransform,c);

				glBindTexture(GL_TEXTURE_2D,bound_textures[c]);
			
				/* do the texture coordinate stuff */
				if (myTCnode->_nodeType == NODE_TextureCoordinate) {
					glTexCoordPointer (2,GL_FLOAT,0,myTCnode->__compiledpoint.p);
				} else if (myTCnode->_nodeType == NODE_TextureCoordinateGenerator) {
					setupTexGen ((struct X3D_TextureCoordinateGenerator*) myTCnode);
				}
			}
		}
	}
}

void haveTexCoordGenerator (struct X3D_IndexedFaceSet *texC, struct X3D_TextureCoordinate *myTCnode) {
	int c;

	#ifdef TEXVERBOSE
	printf ("have a NODE_TextureCoordinateGenerator\n");
	#endif
		
	/* render the TextureCoordinate node for every texture in this node */
	for (c=0; c<texture_count; c++) {
		render_node (texC->texCoord);
		/* are we ok with this texture yet? */
		if (bound_textures[c] != 0) {
			if (setActiveTexture(c,1.0)) {
	       			if (this_textureTransform) start_textureTransform(this_textureTransform,c);
				glBindTexture(GL_TEXTURE_2D,bound_textures[c]);

				setupTexGen((struct X3D_TextureCoordinateGenerator*) myTCnode);
			}
		}
	}
}
