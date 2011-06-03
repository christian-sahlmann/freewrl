/*
=INSERT_TEMPLATE_HERE=

$Id: Textures.h,v 1.26 2011/06/03 19:20:48 dug9 Exp $

Screen snapshot.

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


#ifndef __FREEWRL_TEXTURES_H__
#define __FREEWRL_TEXTURES_H__


#define TEXTURE_INVALID 0

/* Texture loading table :
   newer Texture handling procedures
   each texture has this kind of structure
*/
struct textureTableIndexStruct {
	struct X3D_Node*	scenegraphNode;
	int    nodeType;
	int    imageType;
	int    status;
	int    hasAlpha;
	GLuint OpenGLTexture;
	int    frames;
	char   *filename;
    int    x;
    int    y;
    unsigned char *texdata;
    GLint  Src;
    GLint  Trc;
};
typedef struct textureTableIndexStruct textureTableIndexStruct_s;

extern textureTableIndexStruct_s* loadThisTexture;
extern GLuint defaultBlankTexture;

/* Vertex Array to Vertex Buffer Object migration - used to have a passedInGenTex() 
   when we had (for instance) Cone textures - put this as part of the VBO. */

struct textureVertexInfo {
	GLfloat	*VA_arrays;
	GLint TC_size; 		/* glTexCoordPointer - size param */
	GLenum TC_type;		/* glTexCoordPointer - type param */	
	GLsizei TC_stride;	/* glTexCoordPointer - stride param */
	GLvoid *TC_pointer;	/* glTexCoordPointer - pointer to first element */
};


/* imageType */
#define PNGTexture 200
#define JPGTexture 300

/* removed from GET_THIS_TEXTURE 
                        } else if (thisTextureType==NODE_GeneratedCubeMapTexture){ 
                                gct = (struct X3D_GeneratedCubeMapTexture*) node; 
                                thisTexture = mt->__textureTableIndex; 

*/


#define GET_THIS_TEXTURE thisTextureType = node->_nodeType; \
                                if (thisTextureType==NODE_ImageTexture){ \
                                it = (struct X3D_ImageTexture*) node; \
                                thisTexture = it->__textureTableIndex; \
                        } else if (thisTextureType==NODE_PixelTexture){ \
                                pt = (struct X3D_PixelTexture*) node; \
                                thisTexture = pt->__textureTableIndex; \
                        } else if (thisTextureType==NODE_MovieTexture){ \
                                mt = (struct X3D_MovieTexture*) node; \
                                thisTexture = mt->__textureTableIndex; \
                        } else if (thisTextureType==NODE_VRML1_Texture2){ \
                                v1t = (struct X3D_VRML1_Texture2*) node; \
                                thisTexture = v1t->__textureTableIndex; \
                        } else if (thisTextureType==NODE_ImageCubeMapTexture){ \
                                ict = (struct X3D_ImageCubeMapTexture*) node; \
                                thisTexture = ict->__textureTableIndex; \
                        } else { ConsoleMessage ("Invalid type for texture, %s\n",stringNodeType(thisTextureType)); return;}

/* for texIsloaded structure */
#define TEX_NOTLOADED       0
#define TEX_LOADING         1
#define TEX_NEEDSBINDING    2
#define TEX_LOADED          3
#define TEX_UNSQUASHED      4

const char *texst(int num);


struct multiTexParams {
	GLint texture_env_mode;
	GLint combine_rgb;
	GLint source0_rgb;
	GLint operand0_rgb;
	GLint source1_rgb;
	GLint operand1_rgb;
	GLint combine_alpha;
	GLint source0_alpha;
	GLint operand0_alpha;
	GLint source1_alpha;
	GLint operand1_alpha;
	GLfloat rgb_scale;
	GLfloat alpha_scale;
};


/* do we have to do textures?? */
#define HAVETODOTEXTURES (textureStackTop != 0)

/* multitexture and single texture handling */
#define MAX_MULTITEXTURE 10

/* texture stuff - see code. Need array because of MultiTextures */
/* first, how many textures do we have? 0 -> MAX_MULTITEXTURE */
extern int textureStackTop; 

/* what are the textures, and what are the multitexturing parameters if more than one? */
//extern struct multiTexParams *textureParameterStack[];
//extern void *textureParameterStack[];
extern GLuint boundTextureStack[]; /* defined as MAX_MULTITEXTURE in size */

extern GLuint     *global_tcin;
extern int     global_tcin_count; 
extern void 	*global_tcin_lastParent;

extern void textureDraw_start(struct X3D_Node *texC, struct textureVertexInfo *tex);
extern void textureDraw_end(void);

extern struct X3D_Node *this_textureTransform;  /* do we have some kind of textureTransform? */

extern int fwl_isTextureLoaded(int texno);
extern int isTextureAlpha(int n);
extern int display_status;


/* appearance does material depending on last texture depth */
#define NOTEXTURE 0
#define TEXTURE_NO_ALPHA 1
#define TEXTURE_ALPHA 2

extern int last_texture_type;
void loadTextureNode (struct X3D_Node *node, struct multiTexParams *param);



void bind_image(int type, struct Uni_String *parenturl, struct Multi_String url,
				GLuint *texture_num,
				int repeatS,
				int repeatT,
				void  *param);

/* other function protos */
void init_multitexture_handling(void);


#endif /* __FREEWRL_TEXTURES_H__ */
