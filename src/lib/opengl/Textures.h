/*
=INSERT_TEMPLATE_HERE=

$Id: Textures.h,v 1.2 2008/11/27 00:27:18 couannette Exp $

Screen snapshot.

*/

#ifndef __FREEX3D_TEXTURES_H__
#define __FREEX3D_TEXTURES_H__


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
                        } else { ConsoleMessage ("Invalid type for texture, %s\n",stringNodeType(thisTextureType)); return;}

/* for texIsloaded structure */
#define TEX_NOTLOADED       0
#define TEX_LOADING         1
#define TEX_NEEDSBINDING	2
#define TEX_LOADED          3
#define TEX_UNSQUASHED      4


/* older stuff - check if needed */

/* bind_texture stores the param table pointer for the texture here */

struct loadTexParams {
	/* data sent in to texture parsing thread */
	GLuint *texture_num;
	GLuint genned_texture;
	unsigned repeatS;
	unsigned repeatT;
	struct Uni_String *parenturl;
	unsigned type;
	struct Multi_String url;

	/* data returned from texture parsing thread */
	char *filename;
	int depth;
	int x;
	int y;
	int frames;		/* 1 unless video stream */
	unsigned char *texdata;
	GLint Src;
	GLint Trc;
	GLint Image;
};

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


#endif /* __FREEX3D_TEXTURES_H__ */
