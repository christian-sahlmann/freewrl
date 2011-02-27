/*
=INSERT_TEMPLATE_HERE=

$Id: Component_CubeMapTexturing.c,v 1.25 2011/02/27 00:07:32 crc_canada Exp $

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
#include "../opengl/OpenGL_Utils.h"
#include "../scenegraph/Component_Shape.h"
#include "../scenegraph/Component_CubeMapTexturing.h"
#include "../input/EAIHelpers.h"
/* #include <GL/glext.h> should be in display.h */
#include "../vrml_parser/CParseGeneral.h" /* for union anyVrml */
#include "../world_script/JScript.h" /* for uint32 typedef */


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
#ifndef IPHONE 
	int count;
	struct X3D_Node *thistex = 0;

        /* printf ("render_ComposedCubeMapTexture, global Transparency %f\n",globalappearanceProperties.transparency); */
	for (count=0; count<6; count++) {

		/* set up the appearanceProperties to indicate a CubeMap */
		appearanceProperties.cubeFace = GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT+count;

		/* go through these, right left, top, bottom, back, front */
		switch (count) {
			case 2: {POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->top,thistex);  break;}
			case 3: {POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->bottom,thistex);   break;}

			case 1: {POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->left,thistex);    break;}
			case 0: {POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->right,thistex); break;}



			case 4: {POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->back,thistex);  break;}
			case 5: {POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->front,thistex);   break;}
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
#endif /* IPHONE */
}

/****************************************************************************
 *
 * GeneratedCubeMapTextures
 *
 ****************************************************************************/


/* is this a DDS file? If so, get it, and subdivide it. Ignore MIPMAPS for now */
/* see: http://www.mindcontrol.org/~hplus/graphics/dds-info/MyDDS.cpp */
/* see: http://msdn.microsoft.com/en-us/library/bb943991.aspx/ */

struct DdsLoadInfo {
  bool compressed;
  bool swap;
  bool palette;
  unsigned int divSize;
  unsigned int blockBytes;
  GLenum internalFormat;
  GLenum externalFormat;
  GLenum type;
};

struct DdsLoadInfo loadInfoDXT1 = {
  true, false, false, 4, 8, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
};
struct DdsLoadInfo loadInfoDXT3 = {
  true, false, false, 4, 16, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
};
struct DdsLoadInfo loadInfoDXT5 = {
  true, false, false, 4, 16, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
};
struct DdsLoadInfo loadInfoBGRA8 = {
  false, false, false, 1, 4, GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE
};
struct DdsLoadInfo loadInfoRGB8 = {
  false, false, false, 1, 3, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE
};
struct DdsLoadInfo loadInfoBGR8 = {
  false, false, false, 1, 3, GL_RGB8, GL_BGR, GL_UNSIGNED_BYTE
};
struct DdsLoadInfo loadInfoBGR5A1 = {
  false, true, false, 1, 2, GL_RGB5_A1, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV
};
struct DdsLoadInfo loadInfoBGR565 = {
  false, true, false, 1, 2, GL_RGB5, GL_RGB, GL_UNSIGNED_SHORT_5_6_5
};
struct DdsLoadInfo loadInfoIndex8 = {
  false, false, true, 1, 1, GL_RGB8, GL_BGRA, GL_UNSIGNED_BYTE
};

bool textureIsDDS(textureTableIndexStruct_s* this_tex, char *filename) {
#ifndef IPHONE 

	FILE *file;
	char *buffer;
	unsigned long fileLen;
union DDS_header hdr;
unsigned int x = 0;
unsigned int y = 0;
unsigned int mipMapCount = 0;
unsigned int size,xSize, ySize;

struct DdsLoadInfo * li;

	printf ("textureIsDDS... node %s, file %s\n",
		stringNodeType(this_tex->scenegraphNode->_nodeType), filename);

	/* read in file */
	file = fopen(filename,"rb");
	if (!file) return FALSE;

	/* have file, read in data */


	/* get file length */
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	/* llocate memory */
	buffer=MALLOC(char *, fileLen+1);
	if (!buffer) {
		fclose(file);
		return FALSE;
	}

	/* read file */
	fread(buffer, fileLen, 1, file);
	fclose(file);

	/* check to see if this could be a valid DDS file */
	if (fileLen < sizeof(hdr)) return FALSE;

	/* look at the header, see what kind of a DDS file it might be */
	memcpy( &hdr, buffer, sizeof(hdr));

	/* does this start off with "DDS " an so on ?? */
	if ((hdr.dwMagic == DDS_MAGIC) && (hdr.dwSize == 124) &&
		(hdr.dwFlags & DDSD_PIXELFORMAT) && (hdr.dwFlags & DDSD_CAPS)) {
		printf ("matched :DDS :\n");


printf ("dwFlags %x, DDSD_PIXELFORMAT %x, DDSD_CAPS %x\n",hdr.dwFlags, DDSD_PIXELFORMAT, DDSD_CAPS);
  xSize = hdr.dwWidth;
  ySize = hdr.dwHeight;
printf ("size %d, %d\n",xSize, ySize);
/*
  assert( !(xSize & (xSize-1)) );
  assert( !(ySize & (ySize-1)) );
*/

/*
printf ("looking to see what it is...\n");
printf ("DDPF_FOURCC dwFlags %x mask %x, final %x\n",hdr.sPixelFormat.dwFlags,DDPF_FOURCC,hdr.sPixelFormat.dwFlags & DDPF_FOURCC);

printf ("if it is a dwFourCC, %x and %x\n", hdr.sPixelFormat.dwFourCC ,D3DFMT_DXT1);

printf ("dwFlags %x\n",hdr.sPixelFormat.dwFlags);
printf ("dwRGBBitCount %d\n",hdr.sPixelFormat.dwRGBBitCount);
printf ("dwRBitMask %x\n",hdr.sPixelFormat.dwRBitMask);
printf ("dwGBitMask %x\n",hdr.sPixelFormat.dwGBitMask);
printf ("dwBBitMask %x\n",hdr.sPixelFormat.dwBBitMask);
printf ("dwAlphaBitMask %x\n",hdr.sPixelFormat.dwAlphaBitMask);
printf ("dwFlags and DDPF_ALPHAPIXELS... %x\n",DDPF_ALPHAPIXELS & hdr.sPixelFormat.dwFlags);
*/

  if( PF_IS_DXT1( hdr.sPixelFormat ) ) {
    li = &loadInfoDXT1;
  }
  else if( PF_IS_DXT3( hdr.sPixelFormat ) ) {
    li = &loadInfoDXT3;
  }
  else if( PF_IS_DXT5( hdr.sPixelFormat ) ) {
    li = &loadInfoDXT5;
  }
  else if( PF_IS_BGRA8( hdr.sPixelFormat ) ) {
    li = &loadInfoBGRA8;
  }
  else if( PF_IS_RGB8( hdr.sPixelFormat ) ) {
    li = &loadInfoRGB8;
  }
  else if( PF_IS_BGR8( hdr.sPixelFormat ) ) {
    li = &loadInfoBGR8;
  }
  else if( PF_IS_BGR5A1( hdr.sPixelFormat ) ) {
    li = &loadInfoBGR5A1;
  }
  else if( PF_IS_BGR565( hdr.sPixelFormat ) ) {
    li = &loadInfoBGR565;
  }
  else if( PF_IS_INDEX8( hdr.sPixelFormat ) ) {
    li = &loadInfoIndex8;
  }
  else {
printf ("li failure\n");
return FALSE;
  }

  //fixme: do cube maps later
  //fixme: do 3d later
  x = xSize;
  y = ySize;
  mipMapCount = (hdr.dwFlags & DDSD_MIPMAPCOUNT) ? hdr.dwMipMapCount : 1;
printf ("mipMapCount %d\n",mipMapCount);

  if( li->compressed ) {
printf ("compressed\n");
/*
    size_t size = max( li->divSize, x )/li->divSize * max( li->divSize, y )/li->divSize * li->blockBytes;
    assert( size == hdr.dwPitchOrLinearSize );
    assert( hdr.dwFlags & DDSD_LINEARSIZE );
    unsigned char * data = (unsigned char *)malloc( size );
    if( !data ) {
      goto failure;
    }
    format = cFormat = li->internalFormat;
    for( unsigned int ix = 0; ix < mipMapCount; ++ix ) {
      fread( data, 1, size, f );
      glCompressedTexImage2D( GL_TEXTURE_2D, ix, li->internalFormat, x, y, 0, size, data );
      gl->updateError();
      x = (x+1)>>1;
      y = (y+1)>>1;
      size = max( li->divSize, x )/li->divSize * max( li->divSize, y )/li->divSize * li->blockBytes;
    }
    free( data );
*/
  }
  else if( li->palette ) {
printf ("palette\n");
/*
    //  currently, we unpack palette into BGRA
    //  I'm not sure we always get pitch...
    assert( hdr.dwFlags & DDSD_PITCH );
    assert( hdr.sPixelFormat.dwRGBBitCount == 8 );
    size_t size = hdr.dwPitchOrLinearSize * ySize;
    //  And I'm even less sure we don't get padding on the smaller MIP levels...
    assert( size == x * y * li->blockBytes );
    format = li->externalFormat;
    cFormat = li->internalFormat;
    unsigned char * data = (unsigned char *)malloc( size );
    unsigned int palette[ 256 ];
    unsigned int * unpacked = (unsigned int *)malloc( size*sizeof( unsigned int ) );
    fread( palette, 4, 256, f );
    for( unsigned int ix = 0; ix < mipMapCount; ++ix ) {
      fread( data, 1, size, f );
      for( unsigned int zz = 0; zz < size; ++zz ) {
        unpacked[ zz ] = palette[ data[ zz ] ];
      }
      glPixelStorei( GL_UNPACK_ROW_LENGTH, y );
      glTexImage2D( GL_TEXTURE_2D, ix, li->internalFormat, x, y, 0, li->externalFormat, li->type, unpacked );
      gl->updateError();
      x = (x+1)>>1;
      y = (y+1)>>1;
      size = x * y * li->blockBytes;
    }
    free( data );
    free( unpacked );
*/  
  }
  else {
    if( li->swap ) {
printf ("swap\n");

/*
      glPixelStorei( GL_UNPACK_SWAP_BYTES, GL_TRUE );
*/
    }
    size = x * y * li->blockBytes;

printf ("size is %d\n",size);
/*
    format = li->externalFormat;
    cFormat = li->internalFormat;
    unsigned char * data = (unsigned char *)malloc( size );
    //fixme: how are MIP maps stored for 24-bit if pitch != ySize*3 ?
    for( unsigned int ix = 0; ix < mipMapCount; ++ix ) {
      fread( data, 1, size, f );
      glPixelStorei( GL_UNPACK_ROW_LENGTH, y );
      glTexImage2D( GL_TEXTURE_2D, ix, li->internalFormat, x, y, 0, li->externalFormat, li->type, data );
      gl->updateError();
      x = (x+1)>>1;
      y = (y+1)>>1;
      size = x * y * li->blockBytes;
    }
    free( data );
    glPixelStorei( GL_UNPACK_SWAP_BYTES, GL_FALSE );
    gl->updateError();
*/
  }
/*
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipMapCount-1 );
  gl->updateError();

  return true;

failure:
  return false;
}
*/

	} else {
printf ("put in the dummy file here, and call it quits\n");
	}
	FREE_IF_NZ(buffer);
	return FALSE;
#endif /* IPHONE */
}


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

void compile_ImageCubeMapTexture (struct X3D_ImageCubeMapTexture *node) {
	if (node->__subTextures.n == 0) {
		int i;

		/* printf ("changed_ImageCubeMapTexture - creating sub-textures\n"); */
		FREE_IF_NZ(node->__subTextures.p); /* should be NULL, checking */
		node->__subTextures.p = MALLOC(struct X3D_Node  **,  6 * sizeof (struct X3D_PixelTexture *));
		for (i=0; i<6; i++) {
			node->__subTextures.p[i] = createNewX3DNode(NODE_PixelTexture);
		}
		node->__subTextures.n=6;
	}

	/* tell the whole system to re-create the data for these sub-children */
	node->__regenSubTextures = TRUE;
	MARK_NODE_COMPILED
}


void render_ImageCubeMapTexture (struct X3D_ImageCubeMapTexture *node) {
	int count;

	COMPILE_IF_REQUIRED

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
#ifndef IPHONE
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
		struct X3D_PixelTexture *pt = X3D_PIXELTEXTURE(node->__subTextures.p[count]);
		int xSubIndex, ySubIndex;
		int index;

		xSubIndex=offsets[count*2]*size; ySubIndex=offsets[count*2+1]*size;

		/* create the MFInt32 array for this face in the PixelTexture */
		FREE_IF_NZ(pt->image.p);
		pt->image.n = size*size+3;
		pt->image.p = MALLOC(int *, pt->image.n * sizeof (int));
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
#endif /* IPHONE */
}

