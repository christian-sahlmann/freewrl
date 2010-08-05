/*
=INSERT_TEMPLATE_HERE=

$Id: Component_CubeMapTexturing.c,v 1.14 2010/08/05 18:17:44 uid31638 Exp $

X3D Cubemap Texturing Component

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
#include "../scenegraph/Component_Shape.h"


/* testing */

#define CUBE_MAP_SIZE 256

#ifndef GL_EXT_texture_cube_map
# define GL_NORMAL_MAP_EXT                   0x8511
# define GL_REFLECTION_MAP_EXT               0x8512
# define GL_TEXTURE_CUBE_MAP_EXT             0x8513
# define GL_TEXTURE_BINDING_CUBE_MAP_EXT     0x8514
# define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT  0x8515
# define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT  0x8516
# define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT  0x8517
# define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT  0x8518
# define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT  0x8519
# define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT  0x851A
# define GL_PROXY_TEXTURE_CUBE_MAP_EXT       0x851B
# define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT    0x851C
#endif


void render_ComposedCubeMapTexture (struct X3D_ComposedCubeMapTexture *node) {
	int count;
	struct X3D_Node *thistex = 0;

        /* printf ("render_ComposedCubeMapTexture, global Transparency %f\n",globalappearanceProperties.transparency); */
	for (count=0; count<6; count++) {

		/* set up the appearanceProperties to indicate a CubeMap */
		appearanceProperties.cubeFace = GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT+count;

		/* go through these, back, front, top, bottom, right left */
		switch (count) {
			case 2: {POSSIBLE_PROTO_EXPANSION(node->top,thistex);  break;}
			case 3: {POSSIBLE_PROTO_EXPANSION(node->bottom,thistex);   break;}

			case 1: {POSSIBLE_PROTO_EXPANSION(node->left,thistex);    break;}
			case 0: {POSSIBLE_PROTO_EXPANSION(node->right,thistex); break;}



			case 4: {POSSIBLE_PROTO_EXPANSION(node->back,thistex);  break;}
			case 5: {POSSIBLE_PROTO_EXPANSION(node->front,thistex);   break;}
		}
		if (thistex != 0) {
			/* we have an image specified for this face */
			/* the X3D spec says that a X3DTextureNode has to be one of... */
			if ((thistex->_nodeType == NODE_ImageTexture) ||
			    (thistex->_nodeType == NODE_PixelTexture) ||
			    (thistex->_nodeType == NODE_MovieTexture) ||
			    (thistex->_nodeType == NODE_MultiTexture)) {

				textureStackTop = 0;
				/* render the proper texture */
				render_node((void *)thistex);
			} 
		}
	}
}

void render_GeneratedCubeMapTexture (struct X3D_GeneratedCubeMapTexture *node) {
        /* printf ("render_ImageTexture, global Transparency %f\n",appearanceProperties.transparency); */
        loadTextureNode(X3D_NODE(node),NULL);
        textureStackTop=1; /* not multitexture - should have saved to boundTextureStack[0] */
}

void render_ImageCubeMapTexture (struct X3D_ImageCubeMapTexture *node) {
        /* printf ("render_ImageTexture, global Transparency %f\n",appearanceProperties.transparency); */
        loadTextureNode(X3D_NODE(node),NULL);
        textureStackTop=1; /* not multitexture - should have saved to boundTextureStack[0] */
}


void xxxxrender_ComposedCubeMapTexture(struct X3D_ComposedCubeMapTexture *node)
{
	struct X3D_Node *thistex = 0;
	int count;

#if 1
	char *name = NULL;
	/* Michel testing stuff, please ignore :) */
	static int ft = 0;
	if (ft == 0) {
		/* name = X3DParser_getNameFromNode(X3D_NODE(node)); */ /* => will SIGSEGV */
		/* name = parser_getNameFromNode(X3D_NODE(node));    */ /* => idem         */
		DEBUG_MSG("render_ComposedCubeMapTexture: %p (%s) [%s]\n",
			  (void*) node, name ? name : "<node>", stringNodeType(node->_nodeType));
		ft = 1;
	}
#endif

	for (count=0; count<6; count++) {
		/* go through these, back, front, top, bottom, right left */
		switch (count) {
			case 0: {POSSIBLE_PROTO_EXPANSION(node->front,thistex);  break;}
			case 1: {POSSIBLE_PROTO_EXPANSION(node->back,thistex);   break;}
			case 2: {POSSIBLE_PROTO_EXPANSION(node->top,thistex);    break;}
			case 3: {POSSIBLE_PROTO_EXPANSION(node->bottom,thistex); break;}
			case 4: {POSSIBLE_PROTO_EXPANSION(node->right,thistex);  break;}
			case 5: {POSSIBLE_PROTO_EXPANSION(node->left,thistex);   break;}
		}
		if (thistex != 0) {
			/* we have an image specified for this face */
			/* the X3D spec says that a X3DTextureNode has to be one of... */
			if ((thistex->_nodeType == NODE_ImageTexture) ||
			    (thistex->_nodeType == NODE_PixelTexture) ||
			    (thistex->_nodeType == NODE_MovieTexture) ||
			    (thistex->_nodeType == NODE_MultiTexture)) {

				textureStackTop = 0;
				/* render the proper texture */
				render_node((void *)thistex);
			} 
		}
	}
}


