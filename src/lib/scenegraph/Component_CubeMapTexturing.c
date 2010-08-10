/*
=INSERT_TEMPLATE_HERE=

$Id: Component_CubeMapTexturing.c,v 1.15 2010/08/10 21:15:59 crc_canada Exp $

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
#include "../scenegraph/Component_CubeMapTexturing.h"
#include "../input/EAIHelpers.h"


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


/****************************************************************************
 *
 * ComposedCubeMapTextures
 *
 ****************************************************************************/

void render_ComposedCubeMapTexture (struct X3D_ComposedCubeMapTexture *node) {
	int count;
	struct X3D_Node *thistex = 0;

        /* printf ("render_ComposedCubeMapTexture, global Transparency %f\n",globalappearanceProperties.transparency); */
	for (count=0; count<6; count++) {

		/* set up the appearanceProperties to indicate a CubeMap */
		appearanceProperties.cubeFace = GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT+count;

		/* go through these, right left, top, bottom, back, front */
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

/****************************************************************************
 *
 * GeneratedCubeMapTextures
 *
 ****************************************************************************/

void render_GeneratedCubeMapTexture (struct X3D_GeneratedCubeMapTexture *node) {
        /* printf ("render_ImageTexture, global Transparency %f\n",appearanceProperties.transparency); */
        loadTextureNode(X3D_NODE(node),NULL);
        textureStackTop=1; /* not multitexture - should have saved to boundTextureStack[0] */
}


/****************************************************************************
 *
 * ImageCubeMapTextures
 * notes - we make 6 PixelTextures, and actually put the data for each face 
 * into these pixelTextures. 
 *
 * yes, maybe there is a better way; this way mimics the ComposedCubeMap
 * method, and makes rendering both ImageCubeMap and ComposedCubeMapTextures
 * the same, in terms of scene-graph traversal.
 *
 * look carefully at the call to unpackImageCubeMap...
 *
 ****************************************************************************/

void changed_ImageCubeMapTexture (struct X3D_ImageCubeMapTexture *node) {
	if (node->__subTextures.n == 0) {
		int i;

		/* printf ("changed_ImageCubeMapTexture - creating sub-textures\n"); */
		FREE_IF_NZ(node->__subTextures.p); /* should be NULL, checking */
		node->__subTextures.p = MALLOC( 6 * sizeof (struct X3D_PixelTexture *));
		for (i=0; i<6; i++) {
			node->__subTextures.p[i] = createNewX3DNode(NODE_PixelTexture);
			// JAS I dont think this is necessary as you can
			// not route to this node:  ADD_PARENT(node,node->__subTextures.p[i]);
			// and it might create an infinite update loop...
		}
		node->__subTextures.n=6;
	}

	/* tell the whole system to re-create the data for these sub-children */
	node->__regenSubTextures = TRUE;
}


void render_ImageCubeMapTexture (struct X3D_ImageCubeMapTexture *node) {
	int count;

	/* do we have to split this CubeMap raw data apart? */
	if (node->__regenSubTextures) {
		/* Yes! Get the image data from the file, and split it apart */
		loadTextureNode(X3D_NODE(node),NULL);
	} else {
		/* we have the 6 faces from the image, just go through and render them as a cube */
		if (node->__subTextures.n == 0) return; /* not generated yet - see changed_ImageCubeMapTexture */

		for (count=0; count<6; count++) {

			/* set up the appearanceProperties to indicate a CubeMap */
			appearanceProperties.cubeFace = GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT+count;

			/* go through these, back, front, top, bottom, right left */
			render_node(node->__subTextures.p[count]);
		}
	}
}


/* textures - we have got a png (jpeg, etc) file with a cubemap in it; eg, see:
	http://en.wikipedia.org/wiki/Cube_mapping
*/

	/* images are stored in an image as 3 "rows", 4 "columns", we pick the data out of these columns */
static int offsets[]={
	1,2,	/* right 	*/
	1,0,	/* left 	*/
	2,1,	/* top		*/
	0,1,	/* bottom	*/
	1,1,	/* back		*/
	1,3};	/* front	*/

/* or:
	--	Top	--	--
	Left	Front	Right	Back
	--	Down	--	--
*/

/* fill in the 6 PixelTextures from the data in the texture */
void unpackImageCubeMap (textureTableIndexStruct_s* me) {
	int size;
	int count;

	struct X3D_ImageCubeMapTexture *node = (struct X3D_ImageCubeMapTexture *)me->scenegraphNode;

	if (node == NULL) { 
		ERROR_MSG("problem unpacking single image ImageCubeMap\n");
		return; 
	}

	if (node->_nodeType != NODE_ImageCubeMapTexture) {
		ERROR_MSG("internal error - expected ImageCubeMapTexture here");
		return;
	}

	/* expect the cube map to be in a 4:3 ratio */
	/* printf ("size %dx%d, data %p\n",me->x, me->y, me->texdata); */
	if ((me->x * 3) != (me->y*4)) {
		ERROR_MSG ("expect an ImageCubeMap to be in a 4:3 ratio");
		return;
	}

	/* ok, we have, probably, a cube map in the image data. Extract the data and go nuts */
	size = me->x / 4;


	if (node->__subTextures.n != 6) {
		ERROR_MSG("unpackImageCubeMap, there should be 6 PixelTexture nodes here\n");
		return;
	}
	/* go through each face, and send the data to the relevant PixelTexture */
	/* order: right left, top, bottom, back, front */
	for (count=0; count <6; count++) {
		int x,y;
		uint32 val;
		uint32 *tex = (uint32 *) me->texdata;
		struct X3D_PixelTexture *pt = node->__subTextures.p[count];
		int xSubIndex, ySubIndex;
		int index;

		xSubIndex=offsets[count*2]*size; ySubIndex=offsets[count*2+1]*size;

		/* create the MFInt32 array for this face in the PixelTexture */
		FREE_IF_NZ(pt->image.p);
		pt->image.n = size*size+3;
		pt->image.p = MALLOC(pt->image.n * sizeof (int));
		pt->image.p[0] = size;
		pt->image.p[1] = size;
		pt->image.p[2] = 4; /* this last one is for RGBA */
		index = 3;

		for (x=xSubIndex; x<xSubIndex+size; x++) {
			for (y=ySubIndex; y<ySubIndex+size; y++) {
/*
			if the image needs to be reversed, but I dont think it does, use this loop
			for (y=ySubIndex+size-1; y>=ySubIndex; y--) {
*/
				val = tex[x*me->x+y];
				/* remember, this will be in ARGB format, make into RGBA */
				pt->image.p[index] = ((val & 0xffffff) << 8) | ((val & 0xff000000) >> 24); 
				/* printf ("was %x, now %x\n",tex[x*me->x+y], pt->image.p[index]); */
				index ++;
			}

		}
	}

	/* we are now locked-n-loaded */
	node->__regenSubTextures = FALSE;

	/* get rid of the original texture data now */
	FREE_IF_NZ(me->texdata);
}
