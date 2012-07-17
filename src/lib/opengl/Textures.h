/*
=INSERT_TEMPLATE_HERE=

$Id: Textures.h,v 1.36 2012/07/17 17:00:41 crc_canada Exp $

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

//extern textureTableIndexStruct_s* loadThisTexture;
//extern GLuint defaultBlankTexture;

/* Vertex Array to Vertex Buffer Object migration - used to have a passedInGenTex() 
   when we had (for instance) Cone textures - put this as part of the VBO. */

struct textureVertexInfo {
    GLfloat *pre_canned_textureCoords;
	GLint TC_size; 		/* glTexCoordPointer - size param */
	GLenum TC_type;		/* glTexCoordPointer - type param */	
	GLsizei TC_stride;	/* glTexCoordPointer - stride param */
	GLvoid *TC_pointer;	/* glTexCoordPointer - pointer to first element */
};


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


/* do we have to do textures?? */
#define HAVETODOTEXTURES (gglobal()->RenderFuncs.textureStackTop != 0)

extern void textureDraw_start(struct textureVertexInfo *tex);
extern void textureDraw_end(void);

struct X3D_Node *getThis_textureTransform();

extern int fwl_isTextureLoaded(int texno);
extern int isTextureAlpha(int n);
extern int display_status;


/* appearance does material depending on last texture depth */
#define NOTEXTURE 0
#define TEXTURE_NO_ALPHA 1
#define TEXTURE_ALPHA 2

void loadTextureNode (struct X3D_Node *node, struct multiTexParams *param);
void bind_image(int type, struct Uni_String *parenturl, struct Multi_String url,
				GLuint *texture_num,
				int repeatS,
				int repeatT,
				void  *param);

/* other function protos */
void init_multitexture_handling(void);


#endif /* __FREEWRL_TEXTURES_H__ */
