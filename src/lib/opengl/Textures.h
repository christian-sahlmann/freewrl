/*
=INSERT_TEMPLATE_HERE=

$Id: Textures.h,v 1.12 2009/11/26 21:13:58 crc_canada Exp $

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
	struct	X3D_Node*	scenegraphNode;
	int			nodeType;
	int	imageType;
	int 	status;
	int	depth;
	int 	hasAlpha;
	GLuint	OpenGLTexture;
	int	frames;
	char    *filename;
        int x;
        int y;
        unsigned char *texdata;
	struct Multi_Int32 *pixelData;
        GLint Src;
        GLint Trc;
};

extern struct textureTableIndexStruct* loadThisTexture;
extern GLuint defaultBlankTexture;

/* imageType */
#define PNGTexture 200
#define JPGTexture 300

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


/* we keep track of which textures have been loaded, and which have not */

void bind_image(int type, struct Uni_String *parenturl, struct Multi_String url,
				GLuint *texture_num,
				int repeatS,
				int repeatT,
				void  *param);

/* other function protos */
void init_multitexture_handling(void);


#endif /* __FREEWRL_TEXTURES_H__ */
