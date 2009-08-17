/*
=INSERT_TEMPLATE_HERE=

$Id: Textures.h,v 1.7 2009/08/17 22:25:58 couannette Exp $

Screen snapshot.

*/

#ifndef __FREEWRL_TEXTURES_H__
#define __FREEWRL_TEXTURES_H__


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
	GLuint	*OpenGLTexture;
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
#define TEX_NEEDSBINDING	2
#define TEX_LOADED          3
#define TEX_UNSQUASHED      4


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
